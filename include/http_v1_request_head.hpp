#pragma once

#ifndef MANIFOLD_HTTP_V1_REQUEST_HEAD_HPP
#define MANIFOLD_HTTP_V1_REQUEST_HEAD_HPP

#include "http_v1_message_head.hpp"
#include "http_request_head.hpp"

namespace manifold
{
  namespace http
  {
    //================================================================//
    class v1_request_head : public v1_message_head
    {
    private:
    public:
      //----------------------------------------------------------------//
      v1_request_head(v1_message_head&& hb);
      v1_request_head(const request_head& generic_head);
      v1_request_head(const std::string& url = "/", const std::string& method = "get", std::list<std::pair<std::string,std::string>>&& headers = {});
      v1_request_head(const std::string& url, http::method meth, std::list<std::pair<std::string,std::string>>&& headers = {});
      ~v1_request_head();
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
  }
}

#endif // MANIFOLD_HTTP_V1_REQUEST_HEAD_HPP
