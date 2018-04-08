// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "manifold/http_request_head.hpp"

namespace manifold
{
  namespace http
  {
    //----------------------------------------------------------------//
    static const std::string base64_chars =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    static inline bool is_base64(unsigned char const c)
    {
      return (isalnum(c) || (c == '+') || (c == '/'));
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    std::string base64_encode(unsigned char const* bytes_to_encode, size_t in_len)
    {
      std::string ret;
      size_t i = 0;

      unsigned char char_array_3[3];
      memset(char_array_3, 0, 3);
      unsigned char char_array_4[4];
      memset(char_array_4, 0, 4);

      while (in_len--)
      {
        char_array_3[i++] = *(bytes_to_encode++);
        if (i == 3)
        {
          char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
          char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
          char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
          char_array_4[3] = char_array_3[2] & 0x3f;

          for(i = 0; (i <4) ; i++)
            ret += base64_chars[char_array_4[i]];
          i = 0;
        }
      }

      if (i)
      {
        for(size_t j = i; j < 3; j++)
          char_array_3[j] = '\0';

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for (size_t j = 0; (j < i + 1); j++)
          ret += base64_chars[char_array_4[j]];

        while((i++ < 3))
          ret += '=';

      }

      return ret;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    std::string base64_encode(const std::string& data)
    {
      return base64_encode(reinterpret_cast<unsigned char const*>(data.data()), data.size());
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    std::string base64_decode(std::string const& encoded_string) {
      size_t in_len = encoded_string.size();
      size_t i = 0;
      int in_ = 0;
      unsigned char char_array_4[4], char_array_3[3];
      memset(char_array_3, 0, 3);
      memset(char_array_4, 0, 4);
      std::string ret;

      while (in_len-- && ( encoded_string[in_] != '=') && is_base64(encoded_string[in_]))
      {
        char_array_4[i++] = encoded_string[in_]; in_++;
        if (i ==4) {
          for (i = 0; i <4; i++)
            char_array_4[i] = static_cast<unsigned char>(base64_chars.find(char_array_4[i]));

          char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
          char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
          char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

          for (i = 0; (i < 3); i++)
            ret += char_array_3[i];
          i = 0;
        }
      }

      if (i)
      {
        for (size_t j = i; j <4; j++)
          char_array_4[j] = 0;

        for (size_t j = 0; j <4; j++)
          char_array_4[j] = static_cast<unsigned char>(base64_chars.find(char_array_4[j]));

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for (size_t j = 0; (j < i - 1); j++) ret += char_array_3[j];
      }

      return ret;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    std::string basic_auth(const std::string& username, const std::string& password)
    {
      return ("Basic " + base64_encode(username + ":" + password));
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    std::string method_enum_to_string(method m)
    {
      std::string ret;

      if (m == method::head)         ret = "HEAD";
      else if (m == method::get)     ret = "GET";
      else if (m == method::post)    ret = "POST";
      else if (m == method::put)     ret = "PUT";
      else if (m == method::del)     ret = "DELETE";
      else if (m == method::options) ret = "OPTIONS";
      else if (m == method::trace)   ret = "TRACE";
      else if (m == method::connect) ret = "CONNECT";
      else if (m == method::patch)   ret = "PATCH";

      return ret;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    request_head::request_head(header_block&& hb)
      : header_block(std::move(hb))
    {
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    request_head::request_head(const request_head& generic_head)
      : header_block(generic_head)
    {
      this->path(generic_head.path());
      this->method(generic_head.method());
      this->authority(generic_head.authority());
      this->scheme(generic_head.scheme());
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
    const std::string& request_head::method() const
    {
      return this->header(":method");
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void request_head::method(const std::string& value)
    {
      std::string tmp(value);
      std::transform(tmp.begin(), tmp.end(), tmp.begin(), ::toupper);
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
    const std::string& request_head::path() const
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
    const std::string& request_head::scheme() const
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
    const std::string& request_head::authority() const
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