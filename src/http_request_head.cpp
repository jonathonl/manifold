
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
  }
}