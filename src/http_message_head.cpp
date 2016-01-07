//
// Created by Jonathon LeFaive on 1/6/16.
//

#include "http_message_head.hpp"

#include <algorithm>
#include <system_error>

#include "http_message_head.hpp"

namespace manifold
{
  namespace http
  {
    //----------------------------------------------------------------//
    header_block::header_block()
    {

    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    header_block::~header_block()
    {
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
      std::for_each(name.begin(), name.end(), ::tolower);

      for (auto it = this->headers_.begin(); it != this->headers_.end();)
      {
        if (it->first == name)
          it = this->headers_.erase(it);
        else
          ++it;
      }
      this->headers_.push_back(std::pair<std::string,std::string>(std::move(name), std::move(value)));
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
      std::for_each(name.begin(), name.end(), ::tolower);

      for (auto it = this->headers_.begin(); it != this->headers_.end();)
      {
        if (it->first == name)
          it = this->headers_.erase(it);
        else
          ++it;
      }

      std::for_each(values.begin(), values.end(), [this, &whitespace, &name](std::string& value)
      {
        value.erase(0, value.find_first_not_of(whitespace));
        value.erase(value.find_last_not_of(whitespace)+1);

        this->headers_.push_back(std::pair<std::string,std::string>(std::move(name), std::move(value)));
      });

    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    std::string header_block::header(const std::string& name) const
    {
      std::string ret;
      std::string nameToLower(name);
      std::for_each(nameToLower.begin(), nameToLower.end(), ::tolower);

      for (auto it = this->headers_.rbegin(); ret.empty() && it != this->headers_.rend(); ++it)
      {
        if (it->first == nameToLower)
          ret = it->second;
      }

      return ret;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    std::list<std::string> header_block::multi_header(const std::string& name) const
    {
      std::list<std::string> ret;
      std::string nameToLower(name);
      std::for_each(nameToLower.begin(), nameToLower.end(), ::tolower);

      for (auto it = this->headers_.begin(); it != this->headers_.end(); ++it)
      {
        if (it->first == nameToLower)
          ret.push_back(it->second);
      }

      return ret;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    const std::list<std::pair<std::string,std::string>>& header_block::raw_headers() const
    {
      return this->headers_;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    std::size_t header_block::size() const
    {
      return this->headers_.size();
    }

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
  }
}