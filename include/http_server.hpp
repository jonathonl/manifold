#pragma once

#ifndef MANIFOLD_HTTP_SERVER_HPP
#define MANIFOLD_HTTP_SERVER_HPP

#include <regex>
#include <functional>
#include <set>
#include <memory>

#include "asio.hpp"
#include "http_request_head.hpp"
#include "http_response_head.hpp"
#include "http_outgoing_message.hpp"
#include "http_incoming_message.hpp"
#include "http_connection.hpp"

namespace manifold
{
  namespace http
  {
    //================================================================//
    class server
    {
    public:
      //================================================================//
      class request : public incoming_message
      {
      private:
        //----------------------------------------------------------------//
        request_head head_;
        //----------------------------------------------------------------//
      public:
        //----------------------------------------------------------------//
        request(request_head&& head, const std::shared_ptr<http::connection>& conn, std::int32_t stream_id);
        ~request();
        //----------------------------------------------------------------//

        //----------------------------------------------------------------//
        const request_head& head() const;
        //----------------------------------------------------------------//
      };
      //================================================================//

      //================================================================//
      class response : public outgoing_message
      {
      private:
        //----------------------------------------------------------------//
        response_head head_;
        //----------------------------------------------------------------//
      public:
        //----------------------------------------------------------------//
        response(response_head&& head, const std::shared_ptr<http::connection>& conn, std::int32_t stream_id);
        ~response();
        //----------------------------------------------------------------//

        //----------------------------------------------------------------//
        response_head& head();
        //bool end() { return false; }
        //----------------------------------------------------------------//
      };
      //================================================================//
    private:
      //----------------------------------------------------------------//
      asio::io_service& io_service_;
      asio::ip::tcp::acceptor acceptor_;
      asio::ip::tcp::socket socket_;
      std::set<std::shared_ptr<http::connection>> connections_;
      std::list<std::pair<std::regex,std::function<void(server::request&& req, server::response&& res)>>> stream_handlers_;
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      void accept();
      void manage_connection(const std::shared_ptr<http::connection>& conn);
      //----------------------------------------------------------------//
    public:
      //----------------------------------------------------------------//
      server(asio::io_service& ioService);
      ~server();
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      void listen(unsigned short port, const std::string& host = "");
      void register_handler(const std::regex& expression, const std::function<void(server::request&& req, server::response&& res)>& handler);
      //----------------------------------------------------------------//
    };
    //================================================================//
  }
}

#endif // MANIFOLD_HTTP_SERVER_HPP