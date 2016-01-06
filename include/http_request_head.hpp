#pragma once

#ifndef MANIFOLD_HTTP_REQUEST_HEAD_HPP
#define MANIFOLD_HTTP_REQUEST_HEAD_HPP

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
    public:
      //----------------------------------------------------------------//
      virtual std::string method() const = 0;
      virtual void method(const std::string& value) = 0;
      virtual void method(http::method value) = 0;
      virtual bool method_is(http::method methodToCheck) const;
      virtual std::string path() const = 0;
      virtual void path(const std::string& value) = 0;
      virtual std::string scheme() const = 0;
      virtual void scheme(const std::string& value) = 0;
      virtual std::string authority() const = 0;
      virtual void authority(const std::string& value) = 0;
      //----------------------------------------------------------------//
    };
    //================================================================//
  };
}

#endif // MANIFOLD_HTTP_REQUEST_HEAD_HPP