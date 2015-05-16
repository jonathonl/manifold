#pragma once

#ifndef MANIFOLD_HTTP_REQUEST_HEAD_HPP
#define MANIFOLD_HTTP_REQUEST_HEAD_HPP

#include <system_error>
#include "http_message_head.hpp"

namespace manifold
{
  namespace http
  {
    //================================================================//
    enum class method { head = 1, get, post, put, del, options, trace, connect, patch };
    std::string method_enum_to_string(method method);
    //================================================================//

    //================================================================//
    class request_head : public header_block
    {
    private:
    public:
      //----------------------------------------------------------------//
      request_head(const std::string& url = "/", const std::string& method = "get", std::list<hpack::header_field>&& headers = {});
      request_head(const std::string& url, http::method meth, std::list<hpack::header_field>&& headers = {});
      ~request_head();
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      std::string method() const;
      void method(const std::string& value);
      void method(http::method value);
      bool method_is(http::method methodToCheck) const;
      std::string path() const;
      void path(const std::string& value);
      std::string scheme() const;
      void scheme(const std::string& value);
      std::string authority() const;
      void authority(const std::string& value);
      //----------------------------------------------------------------//
    };
    //================================================================//
  };
}

#endif // MANIFOLD_HTTP_REQUEST_HEAD_HPP