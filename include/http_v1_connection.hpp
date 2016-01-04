#pragma once
#ifndef MANIFOLD_HTTP_V1_CONNECTION_HPP
#define MANIFOLD_HTTP_V1_CONNECTION_HPP

#include "socket.hpp"
#include "http_v1_header_block.hpp"

#include <functional>

namespace manifold
{
  namespace http
  {
    class v1_connection
    {
    public:
      v1_connection(non_tls_socket&& sock)
        : socket_(new non_tls_socket(std::move(sock))), headers_buffer_(8192) {}
      v1_connection(tls_socket&& sock)
        : socket_(new tls_socket(std::move(sock))), headers_buffer_(8192) {}
      virtual ~v1_connection() {}

      void run();

      void on_data(std::uint32_t stream_id, const std::function<void(const char* const buf, std::size_t buf_size)>& fn);
      void on_headers(std::uint32_t stream_id, const std::function<bool(v1_header_block&& headers)>& fn);
      void on_close(std::uint32_t stream_id, const std::function<void(std::uint32_t error_code)>& fn);
      void on_end(std::uint32_t stream_id, const std::function<void()>& fn);
      void on_drain(std::uint32_t stream_id, const std::function<void()>& fn);
    private:
      socket* socket_;
      asio::streambuf headers_buffer_;

      std::function<void(const char* const buf, std::size_t buf_size)> on_data_;
      std::function<bool(v1_header_block&& headers)> on_headers_;
      std::function<void(std::uint32_t error_code)> on_close_;
      std::function<void()> on_end_;
      std::function<void()> on_drain_;

      void run_recv_loop();
      void recv_chunked_encoded_body(){}
      void recv_known_length_body(std::uint64_t content_length);
      void run_send_loop() {}
    };
  }
}

#endif //MANIFOLD_HTTP_V1_CONNECTION_HPP
