
#include <cmath>
#include <assert.h>

#include "hpack.hpp"

namespace std
{
  template <>
  struct hash<std::pair<std::string,std::string>>
  {
    std::size_t operator()(const std::pair<std::string,std::string>& k) const
    {
      // Compute individual hash values for first,
      // second and third and combine them using XOR
      // and bit shifting:

      return ((std::hash<std::string>()(k.first) ^ (std::hash<std::string>()(k.second) << 1)) >> 1);
    }
  };

}

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
    const std::unordered_multimap<std::string, std::size_t> static_table_reverse_lookup_map{
      {":authority"                  ,  1 },  // ""
      {":method"                     ,  2 },  // "GET"
      {":method"                     ,  3 },  // "POST"
      {":path"                       ,  4 },  // "/"
      {":path"                       ,  5 },  // "/index.html"
      {":scheme"                     ,  6 },  // "http"
      {":scheme"                     ,  7 },  // "https"
      {":status"                     ,  8 },  // "200"
      {":status"                     ,  9 },  // "204"
      {":status"                     ,  10},  // "206"
      {":status"                     ,  11},  // "304"
      {":status"                     ,  12},  // "400"
      {":status"                     ,  13},  // "404"
      {":status"                     ,  14},  // "500"
      {"accept-charset"              ,  15},  // ""
      {"accept-encoding"             ,  16},  // "gzip, deflate"
      {"accept-language"             ,  17},  // ""
      {"accept-ranges"               ,  18},  // ""
      {"accept"                      ,  19},  // ""
      {"access-control-allow-origin" ,  20},  // ""
      {"age"                         ,  21},  // ""
      {"allow"                       ,  22},  // ""
      {"authorization"               ,  23},  // ""
      {"cache-control"               ,  24},  // ""
      {"content-disposition"         ,  25},  // ""
      {"content-encoding"            ,  26},  // ""
      {"content-language"            ,  27},  // ""
      {"content-length"              ,  28},  // ""
      {"content-location"            ,  29},  // ""
      {"content-range"               ,  30},  // ""
      {"content-type"                ,  31},  // ""
      {"cookie"                      ,  32},  // ""
      {"date"                        ,  33},  // ""
      {"etag"                        ,  34},  // ""
      {"expect"                      ,  35},  // ""
      {"expires"                     ,  36},  // ""
      {"from"                        ,  37},  // ""
      {"host"                        ,  38},  // ""
      {"if-match"                    ,  39},  // ""
      {"if-modified-since"           ,  40},  // ""
      {"if-none-match"               ,  41},  // ""
      {"if-range"                    ,  42},  // ""
      {"if-unmodified-since"         ,  43},  // ""
      {"last-modified"               ,  44},  // ""
      {"link"                        ,  45},  // ""
      {"location"                    ,  46},  // ""
      {"max-forwards"                ,  47},  // ""
      {"proxy-authenticate"          ,  48},  // ""
      {"proxy-authorization"         ,  49},  // ""
      {"range"                       ,  50},  // ""
      {"referer"                     ,  51},  // ""
      {"refresh"                     ,  52},  // ""
      {"retry-after"                 ,  53},  // ""
      {"server"                      ,  54},  // ""
      {"set-cookie"                  ,  55},  // ""
      {"strict-transport-security"   ,  56},  // ""
      {"transfer-encoding"           ,  57},  // ""
      {"user-agent"                  ,  58},  // ""
      {"vary"                        ,  59},  // ""
      {"via"                         ,  60},  // ""
      {"www-authenticate"            ,  61}}; // ""
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void context::table_evict()
    {
      this->current_dynamic_table_size_ -= (32 + this->dynamic_table_.back().first.size() + this->dynamic_table_.back().second.size());
      this->dynamic_table_.pop_back();
      assert(this->dynamic_table_.size() > 0 || this->current_dynamic_table_size_ == 0);
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void context::table_insert(const std::pair<std::string,std::string>& entry)
    {
      this->table_insert(std::pair<std::string,std::string>(entry));
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void context::table_insert(std::pair<std::string,std::string>&& entry)
    {
      std::size_t entry_size = 32 + entry.first.size() + entry.second.size();
      while ((entry_size + this->current_dynamic_table_size_) > this->max_dynamic_table_size_ && this->dynamic_table_.size() != 0)
        this->table_evict();

      // Entrys that are larger than max table size leave the table empty.
      if ((entry_size + this->current_dynamic_table_size_) <= this->max_dynamic_table_size_)
      {
        this->current_dynamic_table_size_ += entry_size;
        this->dynamic_table_.push_front(std::move(entry));
      }
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void encoder::encode_integer(prefix_mask prfx_mask, std::uint64_t input, std::string& output)
    {
      if (input < (std::uint8_t)prfx_mask)
      {
        output.back() |= ((std::uint8_t)prfx_mask & (std::uint8_t)input);
      }
      else
      {
        output.back() |= (std::uint8_t)prfx_mask;

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
    encoder::find_result encoder::find(const header_field& header_to_find)
    {
      find_result ret;
      auto static_table_find_range = static_table_reverse_lookup_map.equal_range(header_to_find.name);
      if (static_table_find_range.first != static_table_find_range.second)
      {
        ret.name_index = static_table_find_range.first->second;
        for( auto it = static_table_find_range.first; ret.name_and_value_index == 0 && it != static_table_find_range.second; ++it)
        {
          if (header_to_find.value == this->at(it->second).second)
          {
            ret.name_and_value_index = it->second;
          }
        }
      }

      if (ret.name_and_value_index == 0)
      {
        std::size_t current_index = static_table.size() + 1;
        for (auto it = this->dynamic_table_.begin(); ret.name_and_value_index == 0 && it != this->dynamic_table_.end(); ++it,++current_index)
        {
          if (it->first == header_to_find.name)
          {
            if (ret.name_index == 0)
            {
              ret.name_index = current_index;
            }

            if (it->second == header_to_find.value)
            {
              ret.name_and_value_index = current_index;
            }
          }
        }
      }

      return ret;
    };
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void encoder::encode(const std::list<header_field>& headers, std::string& output)
    {
      while (this->table_size_updates_.size())
      {
        output.push_back((char)0x20);
        this->max_dynamic_table_size_ = this->table_size_updates_.front();
        encode_integer(prefix_mask::five_bit, this->max_dynamic_table_size_, output);
        while (this->current_dynamic_table_size_ > this->max_dynamic_table_size_)
          this->table_evict();

        this->table_size_updates_.pop();
      }

      for (std::list<header_field>::const_iterator header_itr= headers.begin(); header_itr != headers.end(); ++header_itr)
      {
        find_result res = this->find(*header_itr);

        if (res.name_and_value_index)
        {
          output.push_back((char)0x80);
          encode_integer(prefix_mask::seven_bit, res.name_and_value_index, output);
        }
        else
        {
          prefix_mask pfx_mask;
          if (header_itr->cache == cacheability::yes)
          {
            pfx_mask = prefix_mask::six_bit;
            output.push_back((char)0x40);
            this->table_insert(std::pair<std::string,std::string>(header_itr->name, header_itr->value));
          }
          else if (header_itr->cache == cacheability::never)
          {
            pfx_mask = prefix_mask::four_bit;
            output.push_back((char)0);
          }
          else
          {
            pfx_mask = prefix_mask::four_bit;
            output.push_back((char)0x10);
          }

          if (res.name_index)
          {
            encode_integer(pfx_mask, res.name_index, output);
          }
          else
          {
            output.push_back((char)0x0); // no huffman.
            encode_integer(prefix_mask::seven_bit, header_itr->name.size(), output);
            output += header_itr->name;
          }

          output.push_back((char)0x0); // no huffman.
          encode_integer(prefix_mask::seven_bit, header_itr->value.size(), output);
          output += header_itr->value;

        }
      }
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    // TODO: Decide whether to deal with pos greather than input size.
    std::uint64_t decoder::decode_integer(prefix_mask prfx_mask, std::string::const_iterator& itr)
    {
      auto a = (std::uint8_t)*itr;
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
      ++itr;
      return ret;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    bool decoder::decode_string_literal(std::string::const_iterator& itr, std::string& output)
    {
      bool ret = true;
      bool huffman_encoded = (*itr & 0x80) != 0;
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
    bool decoder::decode_nvp(std::size_t table_index, cacheability cache_header, std::string::const_iterator& itr, std::list<header_field>& headers)
    {
      bool ret = true;
      std::string n;
      std::string v;

      if (table_index)
      {
        if (table_index <= this->header_list_length())
          n = this->at(table_index).first;
        else
          ret = false;
      }
      else
      {
        ret = this->decode_string_literal(itr, n);
      }

      if (ret)
      {
        ret = this->decode_string_literal(itr, v);
        if (ret)
        {
          if (cache_header == cacheability::yes)
            this->table_insert(std::pair<std::string,std::string>(n, v));
          headers.emplace_back(std::move(n), std::move(v), cache_header);
        }
      }

      return ret;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    bool decoder::decode(std::string::const_iterator itr, std::string::const_iterator end, std::list<header_field>& headers)
    {
      bool ret = true;

      while (ret && itr != end)
      {
        if (*itr & 0x80)
        {
          // Indexed Header Field Representation
          //
          std::uint64_t table_index = decode_integer(prefix_mask::seven_bit, itr);
          if (table_index && table_index <= this->header_list_length())
          {
            const std::pair<std::string,std::string>& p = this->at(table_index);
            headers.emplace_back(p.first, p.second);
          }
          else
            ret = false;
        }
        else if ((*itr & 0xC0) == 0x40)
        {
          // Literal Header Field with Incremental Indexing
          //
          std::uint64_t table_index = decode_integer(prefix_mask::six_bit, itr);
          ret = this->decode_nvp(table_index, cacheability::yes, itr, headers);
        }
        else if ((*itr & 0xF0) == 0x0)
        {
          // Literal Header Field without Indexing
          //
          std::uint64_t table_index = decode_integer(prefix_mask::four_bit, itr);
          ret = this->decode_nvp(table_index, cacheability::no, itr, headers);
        }
        else if ((*itr & 0xF0) == 0x10)
        {
          // Literal Header Field never Indexed
          //
          std::uint64_t table_index = decode_integer(prefix_mask::four_bit, itr);
          ret = this->decode_nvp(table_index, cacheability::never, itr, headers);
        }
        else if ((*itr & 0xE0) == 0x20)
        {
          // Dynamic Table Size Update
          //
          std::uint64_t new_max_size = decode_integer(prefix_mask::five_bit, itr);
          this->max_dynamic_table_size_ = new_max_size;
          while (this->current_dynamic_table_size_ > this->max_dynamic_table_size_)
            this->table_evict();
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