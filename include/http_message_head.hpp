#pragma once

#ifndef IPSUITE_HTTP_MESSAGE_HEAD_HPP
#define IPSUITE_HTTP_MESSAGE_HEAD_HPP

#include <string>
#include <list>

#include "socket.hpp"

namespace IPSuite
{
  namespace HTTP
  {
    // Reference URL: http://tools.ietf.org/html/rfc2616#section-4.4

    //================================================================//
    enum class TransferEncoding { Unknown = 0, Identity, Chunked };
    //================================================================//

    //================================================================//
    class MessageHead
    {
    protected:
      //----------------------------------------------------------------//
      std::string version_;
      std::list<std::pair<std::string,std::string>> headers_;
      //----------------------------------------------------------------//
    public:
      //----------------------------------------------------------------//
      MessageHead();
      virtual ~MessageHead();
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      void header(const std::string& name, const std::string& value);
      void header(std::string&& name, std::string&& value);
      void multiHeader(const std::string& name, const std::list<std::string>& values);
      void multiHeader(std::string&& name, std::list<std::string>&& values);
      std::string header(const std::string& name) const;
      std::list<std::string> multiHeader(const std::string& name) const;
      const std::string& httpVersion() const;
      void httpVersion(const std::string& version); // TODO: Make this an enum.
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      virtual void startLine(const std::string& value) = 0;
      virtual std::string startLine() const = 0;
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      static void serialize(const MessageHead& source, std::string& destination);
      static bool deserialize(const std::string& source, MessageHead& destination);
      //----------------------------------------------------------------//
    };
    //================================================================//
  }
}

#endif // IPSUITE_HTTP_MESSAGE_HEAD_HPP