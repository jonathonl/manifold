// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#ifndef MANIFOLD_HTTP_REQUEST_HEAD_HPP
#define MANIFOLD_HTTP_REQUEST_HEAD_HPP

#include "http_header_block.hpp"

namespace manifold
{
  namespace http
  {
    //================================================================//
    std::string basic_auth(const std::string& username, const std::string& password);
    //================================================================//

    //================================================================//
    enum class method { head = 1, get, post, put, del, options, trace, connect, patch };
    std::string method_enum_to_string(method m);
    //================================================================//

    //================================================================//
    class request_head : public header_block
    {
    private:
    public:
      //----------------------------------------------------------------//
      request_head(header_block&& hb);
      request_head(const request_head& generic_head);
      request_head(const std::string& url = "/", const std::string& method = "get", std::list<hpack::header_field>&& headers = {});
      request_head(const std::string& url, http::method meth, std::list<hpack::header_field>&& headers = {});
      ~request_head();
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      const std::string& method() const;
      void method(const std::string& value);
      void method(http::method value);
      const std::string& path() const;
      void path(const std::string& value);
      const std::string& scheme() const;
      void scheme(const std::string& value);
      const std::string& authority() const;
      void authority(const std::string& value);
      //----------------------------------------------------------------//
    };
    //================================================================//
  }
}

#endif // MANIFOLD_HTTP_REQUEST_HEAD_HPP