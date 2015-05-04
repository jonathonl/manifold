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
    class request_head : public message_head
    {
    private:
      //----------------------------------------------------------------//
      std::string method_;
      std::string url_;
      //----------------------------------------------------------------//
    public:
      //----------------------------------------------------------------//
      request_head(const std::string& url = "/", const std::string& method = "get", std::list<std::pair<std::string,std::string>>&& headers = std::list<std::pair<std::string,std::string>>());
      request_head(const std::string& url, http::method meth, std::list<std::pair<std::string,std::string>>&& headers = std::list<std::pair<std::string,std::string>>());
      ~request_head();
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      const std::string& method() const;
      void method(const std::string& value);
      void method(http::method value);
      bool method_is(http::method methodToCheck) const;
      const std::string& url() const;
      void url(const std::string& value);
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      void start_line(const std::string& value);
      std::string start_line() const;
      //----------------------------------------------------------------//
    };
    //================================================================//
  };
}

#endif // MANIFOLD_HTTP_REQUEST_HEAD_HPP