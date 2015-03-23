#pragma once

#ifndef IPSUITE_HTTP_MESSAGE_HPP
#define IPSUITE_HTTP_MESSAGE_HPP

#include "socket.hpp"
#include "http_message_head.hpp"

namespace IPSuite
{
  namespace HTTP
  {
    //================================================================//
    class Message
    {
    public:
      //----------------------------------------------------------------//
      enum class ErrorCode { NoError = 0, SocketError, HeadCorrupt, HeadTooLarge };
      //----------------------------------------------------------------//
    protected:
      //----------------------------------------------------------------//
      MessageHead& head_;
      Socket& socket_;
      TransferEncoding transferEncoding_;
      std::uint64_t contentLength_;
      bool eof_;
      ErrorCode errorCode_;
      //----------------------------------------------------------------//
    public:
      //----------------------------------------------------------------//
      Message(MessageHead& head, Socket& sock);
      virtual ~Message();
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      ErrorCode errorCode() const;
      std::string errorMessage() const;
      bool isGood() const;
      const Socket& socket() const;
      //----------------------------------------------------------------//
    };
    //================================================================//
  }
}

#endif //IPSUITE_HTTP_MESSAGE_HPP
