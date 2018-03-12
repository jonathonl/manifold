#pragma once
#ifndef MANIFOLD_HTTP_V1_CONNECTION_HPP
#define MANIFOLD_HTTP_V1_CONNECTION_HPP

#include "socket.hpp"
#include "http_v1_message_head.hpp"
#include "http_incoming_message.hpp"

#include <asio/spawn.hpp>
#include <boost/coroutine2/all.hpp>

#include <functional>
#include <sstream>
#include <queue>
#include <vector>

namespace manifold
{
  namespace http
  {
    class transaction
    {
    public:
      transaction() :
        coro_{std::bind(coro, this, std::placeholders::_1)}
      {
      }


    private:
      void coro(boost::coroutines2::coroutine<void>::push_type& yield)
      {
      }
    private:
      boost::coroutines2::coroutine<void>::pull_type coro_;
      std::queue<std::vector<char>> incoming_data_chunks_;
    };

    class v1_connection
    {
    public:
      v1_connection(non_tls_socket&& sock) :
        sock_{std::move(sock)}
      {
        asio::spawn(sock_.io_service(), std::bind(recv_loop, this, std::placeholders::_1));
        asio::spawn(sock_.io_service(), std::bind(send_loop, this, std::placeholders::_1));
      }
    private:
      void recv_loop(asio::yield_context yield)
      {
        std::error_code ec;
        while (!ec)
        {
          std::size_t bytes_read = sock_.recvline(recv_buffer_.data(), recv_buffer_.size(), yield[ec]);

          if (ec)
          {

          }
          else
          {
            std::stringstream ss(std::string(recv_buffer_.begin(), recv_buffer_.begin() + bytes_read));
            v1_message_head message_head;
            v1_message_head::deserialize(ss, message_head);


          }
        }
      }

      void send_loop(asio::yield_context yield)
      {
      }
    private:
      non_tls_socket sock_;
      std::array<char, 8192> recv_buffer_;
    };
  }
}

#endif //MANIFOLD_HTTP_V1_CONNECTION_HPP
