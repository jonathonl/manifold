// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <algorithm>
#include <system_error>

#include "manifold/http_header_block.hpp"

namespace manifold
{
  namespace http
  {
    //----------------------------------------------------------------//
    static const std::string empty_string;
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    header_block::header_block()
    {

    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    header_block::header_block(const header_block& other)
    {
      this->headers_ = other.headers_;
//      for (auto it = generic_head.raw_headers().begin(); it != generic_head.raw_headers().end(); ++it)
//        this->header(it->first, it->second);
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    header_block::header_block(std::list<hpack::header_field>&& raw_headers)
      : headers_(std::move(raw_headers))
    {
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    header_block::~header_block()
    {
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void header_block::pseudo_header(const std::string& name, const std::string& value)
    {
      std::string n(name);
      std::string v(value);
      this->pseudo_header(std::move(n), std::move(v));
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void header_block::pseudo_header(std::string&& name, std::string&& value)
    {
      const std::string whitespace(" \t\f\v\r\n");
      value.erase(0, value.find_first_not_of(whitespace));
      value.erase(value.find_last_not_of(whitespace)+1);

      auto first_non_pseudo_header_itr = this->headers_.begin();
      for ( ; first_non_pseudo_header_itr != this->headers_.end(); ++first_non_pseudo_header_itr)
      {
        if (first_non_pseudo_header_itr->name.size() && first_non_pseudo_header_itr->name.front() != ':')
          break;
      }

      for (auto it = this->headers_.begin(); it != first_non_pseudo_header_itr;)
      {
        if (it->name == name)
        {
          this->headers_.erase(it);
          it = first_non_pseudo_header_itr; // Breaking here because multiple psuedo headers prevented.
        }
        else
          ++it;
      }

      this->headers_.insert(first_non_pseudo_header_itr, hpack::header_field(std::move(name), std::move(value)));
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    bool header_block::header_exists(const std::string& name) const
    {
      return this->header_exists(std::string(name));
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    bool header_block::header_exists(std::string&& name) const
    {
      bool ret = false;
      std::transform(name.begin(), name.end(), name.begin(), ::tolower);
      for (auto it = this->headers_.begin(); !ret && it != this->headers_.end(); ++it)
      {
        if (it->name == name)
          ret = true;
      }
      return ret;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void header_block::header(const std::string& name, const std::string& value)
    {
      std::string n(name);
      std::string v(value);
      this->header(std::move(n), std::move(v));
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void header_block::header(std::string&& name, std::string&& value)
    {
      // trim
      const std::string whitespace(" \t\f\v\r\n");
      name.erase(0, name.find_first_not_of(":" + whitespace));
      name.erase(name.find_last_not_of(whitespace)+1);
      value.erase(0, value.find_first_not_of(whitespace));
      value.erase(value.find_last_not_of(whitespace)+1);

      // make name lowercase
      std::transform(name.begin(), name.end(), name.begin(), ::tolower);

      for (auto it = this->headers_.begin(); it != this->headers_.end();)
      {
        if (it->name == name)
          it = this->headers_.erase(it);
        else
          ++it;
      }
      this->headers_.push_back(hpack::header_field(std::move(name), std::move(value)));
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void header_block::multi_header(const std::string& name, const std::list<std::string>& values)
    {
      std::string n(name);
      std::list<std::string> v(values);
      this->multi_header(std::move(n), std::move(v));
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void header_block::multi_header(std::string&& name, std::list<std::string>&& values)
    {
      // trim
      const std::string whitespace(" \t\f\v\r\n");
      name.erase(0, name.find_first_not_of(":" + whitespace));
      name.erase(name.find_last_not_of(whitespace)+1);

      // make name lowercase
      std::transform(name.begin(), name.end(), name.begin(), ::tolower);

      for (auto it = this->headers_.begin(); it != this->headers_.end();)
      {
        if (it->name == name)
          it = this->headers_.erase(it);
        else
          ++it;
      }

      std::for_each(values.begin(), values.end(), [this, &whitespace, &name](std::string& value)
      {
        value.erase(0, value.find_first_not_of(whitespace));
        value.erase(value.find_last_not_of(whitespace)+1);

        this->headers_.push_back(hpack::header_field(std::move(name), std::move(value)));
      });

    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    const std::string& header_block::header(const std::string& name) const
    {
      return this->header(std::string(name));
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    const std::string& header_block::header(std::string&& name) const
    {
      std::transform(name.begin(), name.end(), name.begin(), ::tolower);

      for (auto it = this->headers_.rbegin(); it != this->headers_.rend(); ++it)
      {
        if (it->name == name)
          return  it->value;
      }

      return empty_string;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    std::list<std::string> header_block::multi_header(const std::string& name) const
    {
      return this->multi_header(std::string(name));
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    std::list<std::string> header_block::multi_header(std::string&& name) const
    {
      std::list<std::string> ret;
      std::transform(name.begin(), name.end(), name.begin(), ::tolower);

      for (auto it = this->headers_.begin(); it != this->headers_.end(); ++it)
      {
        if (it->name == name)
          ret.push_back(it->value);
      }

      return ret;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    const std::list<hpack::header_field>& header_block::raw_headers() const
    {
      return this->headers_;
    }
    //----------------------------------------------------------------//

//    //----------------------------------------------------------------//
//    const std::string& header_block::http_version() const
//    {
//      return this->version_;
//    }
//    //----------------------------------------------------------------//
//
//    //----------------------------------------------------------------//
//    void  header_block::http_version(const std::string& version)
//    {
//      this->version_ = version;
//    }
//    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void header_block::serialize(hpack::encoder& enc, const header_block& source, std::string& destination)
    {
      enc.encode(source.headers_, destination);
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    bool header_block::deserialize(hpack::decoder& dec, const std::string& source, header_block& destination)
    {
      return dec.decode(source.begin(), source.end(), destination.headers_);
    }
    //----------------------------------------------------------------//
  }
}