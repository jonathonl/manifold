#pragma once

#ifndef MANIFOLD_HTTP_CLIENT_HPP
#define MANIFOLD_HTTP_CLIENT_HPP

#include <memory>
#include <set>
#include <functional>
#include <initializer_list>
#include <forward_list>
#include <map>
#include <iostream>

#include "http_connection.hpp"
#include "http_request_head.hpp"
#include "http_response_head.hpp"
#include "http_outgoing_message.hpp"
#include "http_incoming_message.hpp"

//class remote_match
//{
//public:
//  remote_match(const std::string& k, const std::string& v) {}
//  remote_match(const std::string& k, std::initializer_list<std::string> or_list) {}
//};
//
//class q
//{
//public:
//  q(std::string tbl) {}
//  q& match(std::initializer_list<remote_match> match_list)
//  {
//    return (*this);
//  }
//};
//
//void foo()
//{
//  //std::map<std::string,std::string> foo = {{"k","v"}};
//
//  //std::string str{std::string("foobar"),std::string("fomanchu")};
//
//  auto a = q("users")
//  .match(
//      {
//          {"last_name","Doe"},
//          {"first_name", {"John", "Jane"}},
//          {"age", {"30","31","32","33","34","35","36","37","38","39","40"}},
//          {"uuid","AF9E783C"}
//      });
//  std::cout << &a << std::endl;
//};

namespace manifold
{
  namespace http
  {
    //================================================================//
    class client
    {
    public:
      typedef std::uint64_t connection_handle;
      //================================================================//
      class response : public incoming_message
      {
      private:
        response_head head_;
      public:
        response(response_head&& head, const std::shared_ptr<http::connection>& conn, std::uint32_t stream_id);
        ~response();

        const response_head& head() const;
      };
      //================================================================//

      //================================================================//
      class request : public outgoing_message
      {
      private:
        request_head head_;
      public:
        request(request_head&& head, const std::shared_ptr<http::connection>& conn, std::uint32_t stream_id);
        ~request();

        request_head& head();

        void on_push_promise(const std::function<void(http::client::request&& request)>& cb);
        void on_response(const std::function<void(http::client::response&& resp)>& cb);
      };
      //================================================================//

      //================================================================//
      struct ssl_options
      {
      public:
        asio::ssl::context::method method;
        ssl_options(asio::ssl::context::method meth = asio::ssl::context::method::sslv2) : method(meth)
        {
        }
      };
      //================================================================//
    private:
      asio::io_service& io_service_;
      asio::ip::tcp::resolver tcp_resolver_;
      std::int32_t last_stream_id_;

      std::unique_ptr<asio::ssl::context> ssl_context_;
      std::shared_ptr<http::connection> connection_;
      std::queue<client::request> waiting_for_connection_queue_;

      std::function<void()> on_connect_;
      std::function<void(const std::error_code& ec)> on_close_;
      std::error_code ec_;

      void resolve_handler(const std::error_code &ec, asio::ip::tcp::resolver::iterator it);
    public:
      client(asio::io_service& ioservice, const std::string& host, short port = 80);
      client(asio::io_service& ioservice, const std::string& host, const ssl_options& options, short port = 443);
      ~client();
      //void connect(std::string& host, std::uint16_t port, const std::function<void(const std::error_code& ec, connection_handle conn)>& cb);
      void on_connect(const std::function<void()>& cb);
      void on_close(const std::function<void(const std::error_code ec)>& cb);
      void make_request(http::request_head&& req_head, const std::function<void(http::client::request&& req)>& cb) {}
      void make_request(const std::function<void(http::client::request&& req)>& cb);

    };
    //================================================================//
  }
}

#endif // MANIFOLD_HTTP_CLIENT_HPP