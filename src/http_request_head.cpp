
#include "http_request_head.hpp"

namespace manifold
{
  namespace http
  {
    //----------------------------------------------------------------//
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

    //----------------------------------------------------------------//
    bool request_head::method_is(http::method methodToCheck) const
    {
      return (this->method() == method_enum_to_string(methodToCheck));
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    request_head::request_head()
    {
      this->method(method::get);
      this->path("/");
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    request_head::~request_head()
    {
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    std::string request_head::method() const
    {
      return this->method_;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void request_head::method(const std::string& value)
    {
      if (value.size())
        this->method_ = value;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void request_head::method(http::method value)
    {
      this->method(method_enum_to_string(value));
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    std::string request_head::path() const
    {
      return this->path_;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void request_head::path(const std::string& value)
    {
      this->path_ = value.size() ? value : "/";
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    std::string request_head::scheme() const
    {
      return this->scheme_;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void request_head::scheme(const std::string& value)
    {
      this->scheme_ = value;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    std::string request_head::authority() const
    {
      return this->authority_;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void request_head::authority(const std::string& value)
    {
      this->authority_ = value;
    }
    //----------------------------------------------------------------//
  }
}