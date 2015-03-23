#pragma once

#ifndef IPSUITE_HTTP_SERVER_HPP
#define IPSUITE_HTTP_SERVER_HPP

#include <regex>
#include <functional>

#include "asio.hpp"
#include "http_request_head.hpp"
#include "http_response_head.hpp"
#include "http_outgoing_message.hpp"
#include "http_incoming_message.hpp"

namespace IPSuite
{
  namespace HTTP
  {
    //================================================================//
    class Server
    {
    public:
      //================================================================//
      class Request : public IncomingMessage
      {
      private:
        RequestHead head_;
      public:
        Request(RequestHead&& head, Socket& sock);
        ~Request();

        const RequestHead& head() const;
      };
      //================================================================//

      //================================================================//
      class Response : public OutgoingMessage
      {
      private:
        ResponseHead head_;
      public:
        Response(ResponseHead&& head, Socket& sock);
        ~Response();

        ResponseHead& head();
      };
      //================================================================//
    private:
      asio::io_service& ioService_;
      asio::ip::tcp::acceptor acceptor_;
      asio::ip::tcp::socket socket_;
      std::set<asio::connection_ptr> connections_;
    public:
      //----------------------------------------------------------------//
      Server(asio::io_service& ioService);
      ~Server();
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      void listen(unsigned short port, const std::string& host = "");
      void accept();
      void registerHandler(const std::regex& expression, std::function<void(Server::Request& req, Server::Response& res)> handler);
      //----------------------------------------------------------------//
    };
    //================================================================//
  }
}

#endif // IPSUITE_HTTP_SERVER_HPP