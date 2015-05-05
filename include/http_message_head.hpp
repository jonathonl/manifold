#pragma once

#ifndef MANIFOLD_HTTP_MESSAGE_HEAD_HPP
#define MANIFOLD_HTTP_MESSAGE_HEAD_HPP

#include <string>
#include <list>

#include "socket.hpp"
#include "hpack.hpp"

namespace manifold
{
  namespace http
  {
    // Reference URL: http://tools.ietf.org/html/rfc2616#section-4.4

    //================================================================//
    enum class transfer_encoding { Unknown = 0, Identity, Chunked };
    //================================================================//

    //================================================================//
    class message_head
    {
    protected:
      //----------------------------------------------------------------//
      std::string version_;
      std::list<std::pair<std::string,std::string>> headers_;
      //----------------------------------------------------------------//
    public:
      //----------------------------------------------------------------//
      message_head();
      virtual ~message_head();
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      void header(const std::string& name, const std::string& value);
      void header(std::string&& name, std::string&& value);
      void multi_header(const std::string& name, const std::list<std::string>& values);
      void multi_header(std::string&& name, std::list<std::string>&& values);
      std::string header(const std::string& name) const;
      std::list<std::string> multi_header(const std::string& name) const;
      const std::string& http_version() const;
      void http_version(const std::string& version); // TODO: Make this an enum.
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      virtual void start_line(const std::string& value) = 0;
      virtual std::string start_line() const = 0;
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      static void serialize(const message_head& source, std::string& destination);
      static bool deserialize(const std::string& source, message_head& destination);
      //----------------------------------------------------------------//
    };
    //================================================================//
  }
}

#endif // MANIFOLD_HTTP_MESSAGE_HEAD_HPP