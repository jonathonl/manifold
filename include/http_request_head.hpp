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
      request_head();
      ~request_head();
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
    private:
      std::string method_;
      std::string path_;
      std::string authority_;
      std::string scheme_;
    };
    //================================================================//
  };
}

#endif // MANIFOLD_HTTP_REQUEST_HEAD_HPP