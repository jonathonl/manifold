#pragma once

#ifndef MANIFOLD_HTTP_SERVER_HPP
#define MANIFOLD_HTTP_SERVER_HPP

#include <regex>
#include <functional>
#include <set>
#include <memory>
#include <ctime>

#include "http_request_head.hpp"
#include "http_response_head.hpp"
#include "http_outgoing_message.hpp"
#include "http_incoming_message.hpp"
#include "http_connection.hpp"
#include "future.hpp"

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
      public:
        //----------------------------------------------------------------//
        request(request_head&& head, const std::shared_ptr<connection::stream>& stream_ptr);
        request(request&& source);
        ~request();
        //----------------------------------------------------------------//

        //----------------------------------------------------------------//
        const request_head& head() const;
        //----------------------------------------------------------------//
      private:
        //----------------------------------------------------------------//
        request_head head_;
        //----------------------------------------------------------------//
      protected:
        //----------------------------------------------------------------//
        header_block& message_head() { return this->head_; }
        //----------------------------------------------------------------//
      };
      //================================================================//

      class push_promise;
      //================================================================//
      class response : public outgoing_message
      {
      public:
        //----------------------------------------------------------------//
        response(response_head&& head, const std::shared_ptr<connection::stream>& stream_ptr, const std::string& request_method, const std::string& request_authority);
        response(response&& source);
        ~response();
        //----------------------------------------------------------------//

        //----------------------------------------------------------------//
        response_head& head();
        future<bool> send_headers(bool end_stream = false);
        push_promise send_push_promise(request_head&& push_promise_headers);
        push_promise send_push_promise(const request_head& push_promise_headers);
        //----------------------------------------------------------------//
      private:
        //----------------------------------------------------------------//
        response_head head_;
        std::string request_method_;
        std::string request_authority_;
        //----------------------------------------------------------------//
      protected:
        //----------------------------------------------------------------//
        header_block& message_head() { return this->head_; }
        //----------------------------------------------------------------//
      };
      //================================================================//

      //================================================================//
      class push_promise
      {
      public:
        push_promise();
        push_promise(request&& req, response&& res);
        void fulfill(const std::function<void(request&& req, response&& res)>& handler);
      private:
        std::unique_ptr<request> req_;
        std::unique_ptr<response> res_;
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
    public:
      //----------------------------------------------------------------//
      server(asio::io_service& ioService, unsigned short port = 80, const std::string& host = "0.0.0.0");
      server(asio::io_service& ioService, asio::ssl::context& ctx, unsigned short port = 443, const std::string& host = "0.0.0.0");
      ~server();
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      void reset_timeout(std::chrono::system_clock::duration value = std::chrono::system_clock::duration::max());

      template <typename Handler>
      void listen(Handler handler);
      template <typename Handler>
      void listen(Handler handler, std::error_code& ec);
      void close();
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

      //----------------------------------------------------------------//
      static std::string date_string(time_t& t)
      {
        const int RFC1123_TIME_LEN = 29;
        struct tm* tm;
        char buf[RFC1123_TIME_LEN+1] = {'\0'};

        tm = std::gmtime(&t);

        strftime(buf, RFC1123_TIME_LEN+1, "%a, %d %b %Y %H:%M:%S GMT", tm);
        return std::string(&buf[0]);
      }
      //----------------------------------------------------------------//
    private:
      future<void> accept();
      future<void> accept(asio::ssl::context& ctx);
      future<void> new_stream_handler(std::shared_ptr<connection::stream> stream_ptr);
    private:
      asio::io_service& io_service_;
      asio::ip::tcp::acceptor acceptor_;
      asio::ssl::context* ssl_context_;
      unsigned short port_;
      std::string host_;
      bool closed_ = false;
      std::set<std::unique_ptr<http::connection>> connections_;
      std::function<future<void>(server::request req, server::response res)> request_handler_;
      std::string default_server_header_ = "Manifold";
      std::chrono::system_clock::duration timeout_;
      //std::list<std::pair<std::regex,std::function<void(server::request&& req, server::response&& res)>>> stream_handlers_;
    };
    //================================================================//

    //----------------------------------------------------------------//
    template <typename Handler>
    void server::listen(Handler handler)
    {
      std::error_code ec;
      this->listen(handler, ec);
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    template <typename Handler>
    void server::listen(Handler handler, std::error_code& ec)
    {
      this->request_handler_ = handler;
      // Open the acceptor with the option to reuse the address (i.e. SO_REUSEADDR).
      //asio::ip::tcp::resolver resolver(io_service_);
      //asio::ip::tcp::endpoint endpoint = *(resolver.resolve({host, std::to_string(port)}));
      auto ep = asio::ip::tcp::endpoint(asio::ip::address::from_string(this->host_), this->port_);

      acceptor_.open(ep.protocol(), ec);
      if (!ec) acceptor_.set_option(asio::ip::tcp::acceptor::reuse_address(true), ec);
      if (!ec) acceptor_.bind(ep, ec);
      if (!ec) acceptor_.listen(asio::socket_base::max_connections, ec);

      if (!ec)
      {
        if (this->ssl_context_)
          this->accept(*this->ssl_context_);
        else
          this->accept();
      }
    }
    //----------------------------------------------------------------//
  }
}

#endif // MANIFOLD_HTTP_SERVER_HPP