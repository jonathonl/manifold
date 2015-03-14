#pragma once

#ifndef IPSUITE_HTTP_INCOMING_MESSAGE_HPP
#define IPSUITE_HTTP_INCOMING_MESSAGE_HPP

#include "socket.hpp"
#include "http_message_head.hpp"

namespace IPSuite
{
  namespace HTTP
  {
    //================================================================//
    class IncomingMessage
    {
    private:
      //----------------------------------------------------------------//
      MessageHead& head_;
      Socket& socket_;
      TransferEncoding transferEncoding_;
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      ssize_t recvChunkedEntity(char* buff, std::size_t buffSize);
      ssize_t recvKnownLengthEntity(char* buff, std::size_t buffSize);
      //----------------------------------------------------------------//
    public:
      //----------------------------------------------------------------//
      //IncomingMessage(const MessageHead& head, Socket&& sock);
      IncomingMessage(MessageHead& head, Socket& sock);
      ~IncomingMessage();
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      ssize_t recv(char* buff, std::size_t buffSize);
      //----------------------------------------------------------------//
    };
    //================================================================//
  }
}

#endif //IPSUITE_HTTP_INCOMING_MESSAGE_HPP
