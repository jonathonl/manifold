
#include <sstream>
#include <algorithm>

#include "http_request_head.hpp"

namespace manifold
{
  namespace http
  {
    std::string method_enum_to_string(method method)
    {
      std::string ret;

      if (method == method::head)         ret = "HEAD";
      else if (method == method::get)     ret = "GET";
      else if (method == method::post)    ret = "POST";
      else if (method == method::put)     ret = "PUT";
      else if (method == method::del)     ret = "DELETE";
      else if (method == method::options) ret = "OPTIONS";
      else if (method == method::trace)   ret = "TRACE";
      else if (method == method::connect) ret = "CONNECT";
      else if (method == method::patch)   ret = "PATCH";

      return ret;
    }

    //----------------------------------------------------------------//
    request_head::request_head()
    {
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    request_head::~request_head()
    {
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    const std::string& request_head::method() const
    {
      return this->method_;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void request_head::method(const std::string& value)
    {
      this->method_ = value;
      std::for_each(this->method_.begin(), this->method_.end(), ::toupper);
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void request_head::method(http::method value)
    {
      this->method_ = method_enum_to_string(value);
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    bool request_head::method_is(http::method methodToCheck) const
    {
      return (this->method_ == method_enum_to_string(methodToCheck));
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    const std::string& request_head::url() const
    {
      return this->url_;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void request_head::url(const std::string& value)
    {
      this->url_ = value;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void request_head::start_line(const std::string& value)
    {
      std::stringstream ss(value);
      std::getline(ss, this->method_, ' ');
      std::getline(ss, this->url_, ' ');
      std::getline(ss, this->version_);
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    std::string request_head::start_line() const
    {
      return this->method_ + " " + this->url_ + " " + this->version_;
    }
    //----------------------------------------------------------------//
  }
}