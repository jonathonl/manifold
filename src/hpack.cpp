
#include <cmath>

#include "hpack.hpp"

namespace manifold
{
  namespace hpack
  {
    //----------------------------------------------------------------//
    const std::array<std::pair<std::string,std::string>, 61> static_table{{
    /* 1  */    {":authority"                  , ""             },
    /* 2  */    {":method"                     , "GET"          },
    /* 3  */    {":method"                     , "POST"         },
    /* 4  */    {":path"                       , "/"            },
    /* 5  */    {":path"                       , "/index.html"  },
    /* 6  */    {":scheme"                     , "http"         },
    /* 7  */    {":scheme"                     , "https"        },
    /* 8  */    {":status"                     , "200"          },
    /* 9  */    {":status"                     , "204"          },
    /* 10 */    {":status"                     , "206"          },
    /* 11 */    {":status"                     , "304"          },
    /* 12 */    {":status"                     , "400"          },
    /* 13 */    {":status"                     , "404"          },
    /* 14 */    {":status"                     , "500"          },
    /* 15 */    {"accept-charset"              , ""             },
    /* 16 */    {"accept-encoding"             , "gzip, deflate"},
    /* 17 */    {"accept-language"             , ""             },
    /* 18 */    {"accept-ranges"               , ""             },
    /* 19 */    {"accept"                      , ""             },
    /* 20 */    {"access-control-allow-origin" , ""             },
    /* 21 */    {"age"                         , ""             },
    /* 22 */    {"allow"                       , ""             },
    /* 23 */    {"authorization"               , ""             },
    /* 24 */    {"cache-control"               , ""             },
    /* 25 */    {"content-disposition"         , ""             },
    /* 26 */    {"content-encoding"            , ""             },
    /* 27 */    {"content-language"            , ""             },
    /* 28 */    {"content-length"              , ""             },
    /* 29 */    {"content-location"            , ""             },
    /* 30 */    {"content-range"               , ""             },
    /* 31 */    {"content-type"                , ""             },
    /* 32 */    {"cookie"                      , ""             },
    /* 33 */    {"date"                        , ""             },
    /* 34 */    {"etag"                        , ""             },
    /* 35 */    {"expect"                      , ""             },
    /* 36 */    {"expires"                     , ""             },
    /* 37 */    {"from"                        , ""             },
    /* 38 */    {"host"                        , ""             },
    /* 39 */    {"if-match"                    , ""             },
    /* 40 */    {"if-modified-since"           , ""             },
    /* 41 */    {"if-none-match"               , ""             },
    /* 42 */    {"if-range"                    , ""             },
    /* 43 */    {"if-unmodified-since"         , ""             },
    /* 44 */    {"last-modified"               , ""             },
    /* 45 */    {"link"                        , ""             },
    /* 46 */    {"location"                    , ""             },
    /* 47 */    {"max-forwards"                , ""             },
    /* 48 */    {"proxy-authenticate"          , ""             },
    /* 49 */    {"proxy-authorization"         , ""             },
    /* 50 */    {"range"                       , ""             },
    /* 51 */    {"referer"                     , ""             },
    /* 52 */    {"refresh"                     , ""             },
    /* 53 */    {"retry-after"                 , ""             },
    /* 54 */    {"server"                      , ""             },
    /* 55 */    {"set-cookie"                  , ""             },
    /* 56 */    {"strict-transport-security"   , ""             },
    /* 57 */    {"transfer-encoding"           , ""             },
    /* 58 */    {"user-agent"                  , ""             },
    /* 59 */    {"vary"                        , ""             },
    /* 60 */    {"via"                         , ""             },
    /* 61 */    {"www-authenticate"            , ""             }}};
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void encoder::encode_integer(prefix_mask prfx_mask, std::uint64_t input, std::string& output)
    {
      if (input < (std::uint8_t)prfx_mask)
      {
        output.push_back((std::uint8_t)prfx_mask & (std::uint8_t)input);
      }
      else
      {
        output.push_back((std::uint8_t)prfx_mask);

        input = input - (std::uint8_t)prfx_mask;

        while (input >= 128)
        {
          output.push_back((std::uint8_t)(input % 128 + 128));
          input = input / 128;
        }

        output.push_back((std::uint8_t)input);
      }

    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void encoder::encode(const std::list<std::pair<std::string,std::string>>& headers, std::string& output)
    {
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    // TODO: Decide whether to deal with pos greather than input size.
    std::uint64_t decoder::decode_integer(prefix_mask prfx_mask, std::string::const_iterator& itr)
    {

      std::uint64_t ret = (std::uint8_t)prfx_mask & *itr;
      if (ret == (std::uint8_t)prfx_mask)
      {
        std::uint64_t m = 0;

        do
        {
          itr++;
          ret = ret + ((*itr & 127) * (std::uint64_t)std::pow(2,m));
          m = m + 7;
        }
        while ((*itr & 128) == 128);
      }
      return ret;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    bool decoder::decode_string_literal(std::string::const_iterator& itr, std::string& output)
    {
      bool ret = true;
      bool huffman_encoded = *itr & 0x80 ? true : false;
      std::size_t name_sz = decode_integer(prefix_mask::seven_bit, itr);
      if (huffman_encoded)
      {
        std::string tmp(itr, itr + name_sz);
        itr += name_sz;
        if (tmp.size() != name_sz)
          ret = false;
        else
          huffman_decode(tmp.begin(), tmp.end(), output);
      }
      else
      {
        output.assign(itr, itr + name_sz);
        itr += name_sz;
        if (output.size() != name_sz)
          ret = false;
      }
      return ret;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    bool decoder::decode_nvp(std::size_t table_index, std::string::const_iterator& itr, std::list<std::pair<std::string,std::string>>& headers)
    {
      bool ret = true;
      std::pair<std::string,std::string> p;
      if (table_index)
      {
        if (table_index < this->combined_table_size())
          p.first = this->at(table_index).first;
        else
          ret = false;
      }
      else
      {
        ret = this->decode_string_literal(itr, p.first);
      }

      if (ret)
      {
        ret = this->decode_string_literal(itr, p.second);
        if (ret)
          headers.push_back(std::move(p));
      }

      return ret;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    bool decoder::decode(std::string::const_iterator itr, std::string::const_iterator end, std::list<std::pair<std::string,std::string>>& headers)
    {
      bool ret = true;

      while (ret && itr != end)
      {
        if (*itr & 0x80)
        {
          // Indexed Header Field Representation
          //
          std::uint64_t table_index = decode_integer(prefix_mask::seven_bit, itr);
          if (table_index < this->combined_table_size())
            headers.push_back(this->at(table_index));
          else
            ret = false;
        }
        else if ((*itr & 0xC0) == 0x40)
        {
          // Literal Header Field with Incremental Indexing
          //
          std::uint64_t table_index = decode_integer(prefix_mask::six_bit, itr);
          ret = this->decode_nvp(table_index, itr, headers);
          // TODO: Insert into dynamic table.
        }
        else if ((*itr & 0xF0) == 0x0)
        {
          // Literal Header Field without Indexing
          //
          std::uint64_t table_index = decode_integer(prefix_mask::four_bit, itr);
          ret = this->decode_nvp(table_index, itr, headers);
        }
        else if ((*itr & 0xF0) == 0x10)
        {
          // Literal Header Field never Indexed
          //
          std::uint64_t table_index = decode_integer(prefix_mask::four_bit, itr);
          ret = this->decode_nvp(table_index, itr, headers);
        }
        else if ((*itr & 0xE0) == 0x20)
        {
          // Dynamic Table Size Update
          //
          std::uint64_t new_max_size = decode_integer(prefix_mask::five_bit, itr);
          // TODO: Set new max size.
        }
        else
        {
          ret = false;
        }
      }

      return ret;
    }
    //----------------------------------------------------------------//
  }
}