#pragma once

#ifndef IPSUITE_HTTP_OUTGOING_MESSAGE_HPP
#define IPSUITE_HTTP_OUTGOING_MESSAGE_HPP

#include "socket.hpp"
#include "http_message_head.hpp"

namespace IPSuite
{
  namespace HTTP
  {
    //================================================================//
    class OutgoingMessage
    {
    private:
      //----------------------------------------------------------------//
      MessageHead& head_;
      Socket& socket_;
      TransferEncoding transferEncoding_;
      std::uint64_t contentLength_;
      std::uint64_t bytesSent_;
      bool headersSent_;
      bool eof_;
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      bool sendChunkedEntity(const char* data, std::size_t dataSize);
      bool sendKnownLengthEntity(const char* data, std::size_t dataSize);
      //----------------------------------------------------------------//
    public:
      //----------------------------------------------------------------//
      OutgoingMessage(MessageHead& head, Socket& sock);
      ~OutgoingMessage();
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      bool sendHead();
      bool send(const char* data, std::size_t dataSize);
      virtual bool end() = 0;
      //----------------------------------------------------------------//
    };
    //================================================================//
  }
}

#endif //IPSUITE_HTTP_OUTGOING_MESSAGE_HPP
