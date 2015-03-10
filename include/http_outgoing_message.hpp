#pragma once

#ifndef IPSUITE_HTTP_OUTGOING_MESSAGE_HPP
#define IPSUITE_HTTP_OUTGOING_MESSAGE_HPP

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
      TransferEncoding transferEncoding_;
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      ssize_t sendChunkedEntity(const char* data, std::size_t dataSize);
      ssize_t sendKnownLengthEntity(const char* data, std::size_t dataSize);
      //----------------------------------------------------------------//
    public:
      //----------------------------------------------------------------//
      //IncomingMessage(const MessageHead& head, Socket&& sock);
      OutgoingMessage(MessageHead& head, Socket&& sock);
      ~OutgoingMessage();
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      ssize_t send(const char* data, std::size_t dataSize);
      bool sendHead();
      void end();
      //----------------------------------------------------------------//
    };
    //================================================================//
  }
}

#endif //IPSUITE_HTTP_OUTGOING_MESSAGE_HPP
