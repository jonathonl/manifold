#pragma once

#ifndef IPSUITE_HTTP_INCOMING_MESSAGE_HPP
#define IPSUITE_HTTP_INCOMING_MESSAGE_HPP

#include "socket.hpp"
#include "http_message.hpp"

namespace IPSuite
{
  namespace HTTP
  {
    //================================================================//
    class IncomingMessage : public Message
    {
    private:
      //----------------------------------------------------------------//
      std::uint64_t bytesReceived_;
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      std::uint64_t bytesRemainingInChunk_;
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      ssize_t recvChunkedEntity(char* buff, std::size_t buffSize);
      ssize_t recvKnownLengthEntity(char* buff, std::size_t buffSize);
      //----------------------------------------------------------------//
    public:
      //----------------------------------------------------------------//
      //IncomingMessage(const MessageHead& head, Socket&& sock);
      IncomingMessage(MessageHead& head, Socket& sock);
      virtual ~IncomingMessage();
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      ssize_t recv(char* buff, std::size_t buffSize);
      //----------------------------------------------------------------//
    };
    //================================================================//
  }
}

#endif //IPSUITE_HTTP_INCOMING_MESSAGE_HPP
