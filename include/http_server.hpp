#pragma once

#ifndef MANIFOLD_HTTP_SERVER_HPP
#define MANIFOLD_HTTP_SERVER_HPP

#include <regex>
#include <functional>
#include <set>
#include <memory>
#include <ctime>

#include "asio.hpp"
#include "http_v2_request_head.hpp"
#include "http_v2_response_head.hpp"
#include "http_outgoing_message.hpp"
#include "http_incoming_message.hpp"
#include "http_v2_connection.hpp"

namespace manifold
{
  namespace http
  {
    //================================================================//
    class server
    {
    private:
      //================================================================//
      class v2_connection : public http::v2_connection<response_head, request_head>
      {
      public:
        v2_connection(non_tls_socket&& sock)
          : http::v2_connection<response_head, request_head>(std::move(sock))
        {
          this->local_settings_[setting_code::enable_push] = 0;
        }
        v2_connection(tls_socket&& sock)
          : http::v2_connection<response_head, request_head>(std::move(sock))
        {
          this->local_settings_[setting_code::enable_push] = 0;
        }
        ~v2_connection() {}
      private:
//        class stream : public http::v2_connection<response_head, request_head>::stream
//        {
//        public:
//          stream(std::uint32_t stream_id, uint32_t initial_window_size, uint32_t initial_peer_window_size) : http::v2_connection<response_head, request_head>::stream(stream_id, initial_window_size, initial_peer_window_size)
//          {
//
//          }
//          ~stream() {}
//
//        private:
//
//        };
//
//        std::uint32_t next_stream_id_;
//
//        stream* create_stream_object(std::uint32_t stream_id = 0)
//        {
//          if (stream_id == 0 && this->next_stream_id_ <= v2_connection::max_stream_id)
//          {
//            stream_id = this->next_stream_id_;
//            this->next_stream_id_ += 2;
//          }
//
//          if (stream_id)
//            return new stream(stream_id, this->local_settings().at(setting_code::initial_window_size), this->peer_settings().at(setting_code::initial_window_size));
//          else
//            return nullptr;
//        }

      };
      //================================================================//
    public:
      //================================================================//
      class request : public incoming_message<response_head, request_head>
      {
      private:
        //----------------------------------------------------------------//
        request_head head_;
        //----------------------------------------------------------------//
      protected:
        //----------------------------------------------------------------//
        //header_block& message_head() { return this->head_; }
        //----------------------------------------------------------------//
      public:
        //----------------------------------------------------------------//
        request(request_head&& head, const std::shared_ptr<http::connection<response_head, request_head>>& conn, std::int32_t stream_id);
        ~request();
        //----------------------------------------------------------------//

        //----------------------------------------------------------------//
        const request_head& head() const;
        //----------------------------------------------------------------//
      };
      //================================================================//

      class push_promise;
      //================================================================//
      class response : public outgoing_message<response_head, request_head>
      {
      private:
        //----------------------------------------------------------------//
        response_head head_;
        //----------------------------------------------------------------//
      protected:
        //----------------------------------------------------------------//
        response_head& message_head() { return this->head_; }
        //----------------------------------------------------------------//
      public:
        //----------------------------------------------------------------//
        response(response_head&& head, const std::shared_ptr<http::connection<response_head, request_head>>& conn, std::int32_t stream_id);
        ~response();
        //----------------------------------------------------------------//

        //----------------------------------------------------------------//
        response_head& head();
        bool send_headers(bool end_stream = false);
        push_promise send_push_promise(request_head&& push_promise_headers);
        push_promise send_push_promise(const request_head& push_promise_headers);
        //----------------------------------------------------------------//
      };
      //================================================================//

      //================================================================//
      class push_promise
      {
      public:
        push_promise(request&& req, response&& res);
        void fulfill(const std::function<void(request&& req, response&& res)>& handler);
      private:
        request req_;
        response res_;
        bool fulfilled_;
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
      std::set<std::shared_ptr<http::v2_connection<response_head, request_head>>> connections_;
      std::function<void(server::request&& req, server::response&& res)> request_handler_;
      std::string default_server_header_ = "Manifold";
      //std::list<std::pair<std::regex,std::function<void(server::request&& req, server::response&& res)>>> stream_handlers_;
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      void accept();
      void accept(asio::ssl::context& ctx);
      void manage_connection(const std::shared_ptr<http::v2_connection<response_head, request_head>>& conn);
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