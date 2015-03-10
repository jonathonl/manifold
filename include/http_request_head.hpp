#pragma once

#ifndef IPSUITE_HTTP_REQUEST_HEAD_HPP
#define IPSUITE_HTTP_REQUEST_HEAD_HPP

#include "http_message_head.hpp"

namespace IPSuite
{
  namespace HTTP
  {
    //================================================================//
    enum class Method { Head = 1, Get, Post, Put, Delete, Options, Trace, Connect, Patch };
    std::string methodEnumToString(Method method);
    //================================================================//

    //================================================================//
    class RequestHead : public MessageHead
    {
    private:
      //----------------------------------------------------------------//
      std::string method_;
      std::string url_;
      //----------------------------------------------------------------//
    public:
      //----------------------------------------------------------------//
      RequestHead();
      ~RequestHead();
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      const std::string& method() const;
      void method(const std::string& value);
      void method(Method value);
      bool methodIs(Method methodToCheck) const;
      const std::string& url() const;
      void url(const std::string& value);
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      void startLine(const std::string& value);
      std::string startLine() const;
      //----------------------------------------------------------------//
    };
    //================================================================//
  };
}

#endif // IPSUITE_HTTP_REQUEST_HEAD_HPP