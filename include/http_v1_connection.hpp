#pragma once
#ifndef MANIFOLD_HTTP_V1_CONNECTION_HPP
#define MANIFOLD_HTTP_V1_CONNECTION_HPP

#include "socket.hpp"
#include "http_v1_message_head.hpp"

#include <functional>
#include <deque>
#include <queue>
#include <memory>

namespace manifold
{
  namespace http
  {
    class v1_connection : public std::enable_shared_from_this<v1_connection>
    {
    public:
      v1_connection(non_tls_socket&& sock)
        : socket_(new non_tls_socket(std::move(sock))), send_loop_running_(false), next_transaction_id_(1) {}
      v1_connection(tls_socket&& sock)
        : socket_(new tls_socket(std::move(sock))), send_loop_running_(false), next_transaction_id_(1) {}
      virtual ~v1_connection() {}

      void run();

      void on_data(const std::function<void(const char* const buf, std::size_t buf_size)>& fn);
      void on_headers(const std::function<void(v1_message_head&& headers)>& fn);
      void on_trailers(const std::function<void(v1_header_block&& trailers)>& fn);
      void on_close(const std::function<void(std::uint32_t error_code)>& fn);
      void on_end(const std::function<void()>& fn);
      void on_drain(const std::function<void()>& fn);

      std::uint64_t start_message();
      void send_message_head(std::uint64_t transaction_id, const v1_message_head& head);
      void send_message_body(std::uint64_t transaction_id, const char*const data, std::size_t data_sz);
      void end_message(std::uint64_t transaction_id, const v1_header_block& trailers = v1_header_block());
    private:
      struct queued_message
      {
        const std::uint64_t id;
        std::string head_data;
        bool head_sent;
        bool chunked_encoding;
        std::uint64_t content_length;
        std::queue<std::vector<char>> body;
        std::string trailer_data;
        bool ended;
        queued_message(std::uint64_t transaction_id)
          : id(transaction_id), head_sent(false), chunked_encoding(false), content_length(0), ended(false) {}
      };

      socket* socket_;
      std::array<char, 8192> recv_buffer_;
      std::deque<queued_message> send_queue_;
      std::uint64_t next_transaction_id_;
      bool send_loop_running_;

      std::function<void(const char* const buf, std::size_t buf_size)> on_data_;
      std::function<void(v1_message_head&& headers)> on_headers_;
      std::function<void(v1_header_block&& trailers)> on_trailers_;
      std::function<void(std::uint32_t error_code)> on_close_;
      std::function<void()> on_end_;
      std::function<void()> on_drain_;

      void run_recv_loop();
      void recv_chunk();
      void recv_trailers();
      void recv_known_length_body(std::uint64_t content_length);
      void run_send_loop();
    };
  }
}

#endif //MANIFOLD_HTTP_V1_CONNECTION_HPP
