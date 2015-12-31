#pragma once

#ifndef MANIFOLD_HTTP_SERVER_HPP
#define MANIFOLD_HTTP_SERVER_HPP

#include <regex>
#include <functional>
#include <set>
#include <memory>
#include <ctime>

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
    private:
      //================================================================//
      class connection : public http::connection
      {
      public:
        connection(non_tls_socket&& sock)
          : http::connection(std::move(sock)) {}
        connection(tls_socket&& sock)
          : http::connection(std::move(sock)) {}
        ~connection() {}
      private:
        class stream : public http::connection::stream
        {
        public:
          stream(std::uint32_t stream_id, uint32_t initial_window_size, uint32_t initial_peer_window_size) : http::connection::stream(stream_id, initial_window_size, initial_peer_window_size)
          {

          }
          ~stream() {}

        private:

        };
        stream* create_stream_object(std::uint32_t stream_id)
        {
          return new stream(stream_id, this->local_settings().at(setting_code::initial_window_size), this->peer_settings().at(setting_code::initial_window_size));
        }

      };
      //================================================================//
    public:
      //================================================================//
      class request : public incoming_message
      {
      private:
        //----------------------------------------------------------------//
        request_head head_;
        //----------------------------------------------------------------//
      protected:
        //----------------------------------------------------------------//
        header_block& message_head() { return this->head_; }
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
      protected:
        //----------------------------------------------------------------//
        header_block& message_head() { return this->head_; }
        //----------------------------------------------------------------//
      public:
        //----------------------------------------------------------------//
        response(response_head&& head, const std::shared_ptr<http::connection>& conn, std::int32_t stream_id);
        ~response();
        //----------------------------------------------------------------//

        //----------------------------------------------------------------//
        response_head& head();
        bool send_headers(bool end_stream = false);
        //----------------------------------------------------------------//
      };
      //================================================================//

      //================================================================//
      struct ssl_options
      {
      public:
        asio::ssl::context::method method;
        std::vector<char> pfx;
        std::vector<char> key;
        std::vector<char> chain;
        std::vector<char> passphrase;
        std::vector<char> cert;
        std::vector<char> ca;
        std::vector<char> dhparam;
        ssl_options(asio::ssl::context::method meth) : method(meth)
        {
        }
      };
      //================================================================//
    private:
      //----------------------------------------------------------------//
      asio::io_service& io_service_;
      asio::ip::tcp::acceptor acceptor_;
      std::unique_ptr<asio::ssl::context> ssl_context_;
      unsigned short port_;
      std::string host_;
      std::set<std::shared_ptr<http::connection>> connections_;
      std::function<void(server::request&& req, server::response&& res)> request_handler_;
      std::string default_server_header_ = "Manifold";
      //std::list<std::pair<std::regex,std::function<void(server::request&& req, server::response&& res)>>> stream_handlers_;
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      void accept();
      void accept(asio::ssl::context& ctx);
      void manage_connection(const std::shared_ptr<http::connection>& conn);
      //----------------------------------------------------------------//
    public:
      //----------------------------------------------------------------//
      server(asio::io_service& ioService, unsigned short port = 80, const std::string& host = "0.0.0.0");
      server(asio::io_service& ioService, ssl_options options, unsigned short port = 443, const std::string& host = "0.0.0.0");
      ~server();
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      void listen(const std::function<void(server::request&& req, server::response&& res)>& handler);
      //void register_handler(const std::regex& expression, const std::function<void(server::request&& req, server::response&& res)>& handler);
      void set_default_server_header(const std::string& value);
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      static std::string date_string()
      {
        const int RFC1123_TIME_LEN = 29;
        time_t t;
        struct tm* tm;
        char buf[RFC1123_TIME_LEN+1] = {'\0'};

        time(&t);
        tm = std::gmtime(&t);

        strftime(buf, RFC1123_TIME_LEN+1, "%a, %d %b %Y %H:%M:%S GMT", tm);
        return std::string(&buf[0]);
      }
      //----------------------------------------------------------------//
    };
    //================================================================//
  }
}

#endif // MANIFOLD_HTTP_SERVER_HPP