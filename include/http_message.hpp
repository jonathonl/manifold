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
    protected:
      //----------------------------------------------------------------//
      MessageHead& head_;
      Socket& socket_;
      TransferEncoding transferEncoding_;
      std::uint64_t contentLength_;
      bool eof_;
      //----------------------------------------------------------------//
    public:
      //----------------------------------------------------------------//
      Message(MessageHead& head, Socket& sock);
      virtual ~Message();
      //----------------------------------------------------------------//
    };
    //================================================================//
  }
}

#endif //IPSUITE_HTTP_MESSAGE_HPP
