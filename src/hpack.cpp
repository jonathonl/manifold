
#include <cmath>

#include "hpack.hpp"

namespace manifold
{
  namespace hpack
  {

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

    void encoder::encode(const std::list<std::pair<std::string,std::string>>& headers, std::string& output)
    {
    }

    // TODO: Decide whether to deal with pos greather than input size.
    std::uint64_t decoder::decode_integer(prefix_mask prfx_mask, const std::string& input, std::size_t& pos)
    {

      std::uint64_t ret = (std::uint8_t)prfx_mask & input[pos];
      if (ret == (std::uint8_t)prfx_mask)
      {
        std::uint64_t m = 0;

        do
        {
          pos++;
          ret = ret + ((input[pos] & 127) * (std::uint64_t)std::pow(2,m));
          m = m + 7;
        }
        while ((input[pos] & 128) == 128);
      }
      return ret;
    }

    bool decoder::decode(const std::string& input, std::list<std::pair<std::string,std::string>>& headers)
    {
      bool ret = true;

      for (std::size_t pos = 0; ret && pos < input.size(); )
      {
        if (input[pos] & 0x80)
        {
          // Indexed Header Field Representation
        }
        else if ((input[pos] & 0xC0) == 0x40)
        {
          // Literal Header Field with Incremental Indexing
        }
        else if ((input[pos] & 0xF0) == 0x0)
        {
          // Literal Header Field without Indexing
        }
        else if ((input[pos] & 0xF0) == 0x10)
        {
          // Literal Header Field never Indexed
        }
        else if ((input[pos] & 0xE0) == 0x20)
        {
          // Dynamic Table Size Update
        }
        else
        {
          ret = false;
        }
      }

      return ret;
    }
  }
}