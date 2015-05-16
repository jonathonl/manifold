
#include <sstream>
#include <algorithm>

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
    request_head::request_head(const std::string& path, const std::string& method, std::list<hpack::header_field>&& headers)
    {
      this->headers_ = std::move(headers);
      this->path(path);
      this->method(method);
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    request_head::request_head(const std::string& path, http::method method, std::list<hpack::header_field>&& headers)
    {
      this->headers_ = std::move(headers);
      this->path(path);
      this->method(method);
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
      return this->header(":method");
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void request_head::method(const std::string& value)
    {
      std::string tmp(value);
      std::for_each(tmp.begin(), tmp.end(), ::toupper);
      this->pseudo_header(":method", std::move(tmp));
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void request_head::method(http::method value)
    {
      this->pseudo_header(":method", std::move(method_enum_to_string(value)));
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    bool request_head::method_is(http::method methodToCheck) const
    {
      return (this->method() == method_enum_to_string(methodToCheck));
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    std::string request_head::path() const
    {
      return this->header(":path");
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void request_head::path(const std::string& value)
    {
      this->pseudo_header(":path", value);
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    std::string request_head::scheme() const
    {
      return this->header(":scheme");
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void request_head::scheme(const std::string& value)
    {
      this->pseudo_header(":scheme", value);
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    std::string request_head::authority() const
    {
      return this->header(":authority");
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void request_head::authority(const std::string& value)
    {
      this->pseudo_header(":authority", value);
    }
    //----------------------------------------------------------------//
  }
}