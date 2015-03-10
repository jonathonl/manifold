#pragma once

#ifndef IPSUITE_HTTP_CLIENT_HPP
#define IPSUITE_HTTP_CLIENT_HPP

#include <cstdint>
#include <string>
#include <list>
#include <sys/socket.h>

#include "http_request_head.hpp"
#include "http_response_head.hpp"
#include "http_outgoing_message.hpp"
#include "http_incoming_message.hpp"

namespace IPSuite
{
  namespace HTTP
  {
    //================================================================//
    class Client
    {
    public:
      //================================================================//
      typedef std::uint64_t ConnectionHandle;
      //================================================================//

      //================================================================//
      class Request : public OutgoingMessage
      {
      private:
        RequestHead head_;
      public:
        Request(RequestHead&& head, Socket&& sock);
        ~Request();

        RequestHead& head();
      };
      //================================================================//

      //================================================================//
      class Response : public IncomingMessage
      {
      private:
        ResponseHead head_;
      public:
        Response(ResponseHead&& head, Socket&& sock);
        ~Response();

        const ResponseHead& head() const;
      };
      //================================================================//
    private:
      std::list<ConnectionHandle> connectionList_;
    public:
      Client();
      ~Client();
    public:
      ConnectionHandle connect(std::string& host, std::uint16_t)
      {
        auto sock = socket(1,2,3);
        return 0;
      }


    };
    //================================================================//
  }
}

#endif // IPSUITE_HTTP_CLIENT_HPP