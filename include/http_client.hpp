#pragma once

#ifndef IPSUITE_HTTP_CLIENT_HPP
#define IPSUITE_HTTP_CLIENT_HPP

#include <cstdint>
#include <string>
#include <list>
#include <functional>

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
      class ConnectionHandle
      {
        friend class Client;
      private:
        std::uint64_t intVal_;
        ConnectionHandle(std::uint64_t intVal);
      public:
        ConnectionHandle();
        ConnectionHandle(const ConnectionHandle& source);
        ConnectionHandle& operator=(const ConnectionHandle& source);
      };
      //================================================================//

      //================================================================//
      class AsyncConnectionHandle
      {
        friend class Client;
      private:
        std::uint64_t intVal_;
        AsyncConnectionHandle(std::uint64_t intVal);
      public:
        AsyncConnectionHandle();
        AsyncConnectionHandle(const ConnectionHandle& source);
        AsyncConnectionHandle& operator=(const ConnectionHandle& source);
      };
      //================================================================//

      //================================================================//
      class Request : public OutgoingMessage
      {
      private:
        RequestHead head_;
      public:
        Request(RequestHead&& head, Socket& sock);
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
        Response(ResponseHead&& head, Socket& sock);
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
        return ConnectionHandle(0);
      }

      void request(ConnectionHandle handle, std::function<void(Request& request)>&& requestFn, std::function<void(Response& response)>&& responseFn);
      void request(AsyncConnectionHandle handle, std::function<void(Request& request)>&& requestFn, std::function<void(Response& response)>&& responseFn);

      void close(ConnectionHandle handle);
      void close(AsyncConnectionHandle handle);
    };
    //================================================================//
  }
}

#endif // IPSUITE_HTTP_CLIENT_HPP