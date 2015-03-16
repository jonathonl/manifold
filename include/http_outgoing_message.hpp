#pragma once

#ifndef IPSUITE_HTTP_OUTGOING_MESSAGE_HPP
#define IPSUITE_HTTP_OUTGOING_MESSAGE_HPP

#include "http_message.hpp"

namespace IPSuite
{
  namespace HTTP
  {
    //================================================================//
    class OutgoingMessage : public Message
    {
    private:
      //----------------------------------------------------------------//
      std::uint64_t bytesSent_;
      bool headersSent_;
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      bool sendChunkedEntity(const char* data, std::size_t dataSize);
      bool sendKnownLengthEntity(const char* data, std::size_t dataSize);
      //----------------------------------------------------------------//
    public:
      //----------------------------------------------------------------//
      OutgoingMessage(MessageHead& head, Socket& sock);
      virtual ~OutgoingMessage();
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
