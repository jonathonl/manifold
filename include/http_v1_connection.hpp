#pragma once
#ifndef MANIFOLD_HTTP_V1_CONNECTION_HPP
#define MANIFOLD_HTTP_V1_CONNECTION_HPP

#include "socket.hpp"
#include "http_connection.hpp"
#include "http_v1_message_head.hpp"

#include <functional>
#include <deque>
#include <queue>
#include <memory>

namespace manifold
{
  namespace http
  {
    template <typename SendMsg, typename RecvMsg>
    class v1_connection : public std::enable_shared_from_this<v1_connection<SendMsg, RecvMsg>>, public connection<SendMsg, RecvMsg>
    {
    public:
      v1_connection(non_tls_socket&& sock)
        : socket_(new non_tls_socket(std::move(sock))),
          closed_(false),
          send_loop_running_(false),
          next_transaction_id_(1) {}
      v1_connection(tls_socket&& sock)
        : socket_(new tls_socket(std::move(sock))),
          closed_(false),
          send_loop_running_(false),
          next_transaction_id_(1) {}
      virtual ~v1_connection() {}

      void run();
      void close(std::uint32_t ec);


      void on_close(const std::function<void(std::uint32_t error_code)>& fn);
      void on_new_stream(const std::function<void(std::uint32_t transaction_id)>& fn);

      void on_headers(std::uint32_t transaction_id, const std::function<void(RecvMsg&& headers)>& fn);
      void on_informational_headers(std::uint32_t transaction_id, const std::function<void(RecvMsg&& headers)>& fn);
      void on_trailers(std::uint32_t transaction_id, const std::function<void(header_block&& headers)>& fn);
      void on_push_promise(std::uint32_t transaction_id, const std::function<void(SendMsg&& headers, std::uint32_t promised_transaction_id)>& fn) {}
      void on_data(std::uint32_t transaction_id, const std::function<void(const char* const buf, std::size_t buf_size)>& fn);
      void on_close(std::uint32_t transaction_id, const std::function<void(std::uint32_t error_code)>& fn);
      void on_end(std::uint32_t transaction_id, const std::function<void()>& fn);
      void on_drain(std::uint32_t transaction_id, const std::function<void()>& fn);


      std::uint32_t create_stream(std::uint32_t dependency_transaction_id, std::uint32_t transaction_id);
      bool send_data(std::uint32_t stream_id, const char *const data, std::uint32_t data_sz, bool end_stream);
      bool send_headers(std::uint32_t stream_id, const SendMsg& head, bool end_headers, bool end_stream);
      bool send_trailers(std::uint32_t stream_id, const header_block& head, bool end_headers, bool end_stream);
      //bool send_headers(std::uint32_t stream_id, const v2_header_block& head, priority_options priority, bool end_headers, bool end_stream);
      //bool send_priority(std::uint32_t stream_id, priority_options options);
      bool send_reset_stream(std::uint32_t stream_id, http::errc error_code)  { this->close((int)error_code); }
      std::uint32_t send_push_promise(std::uint32_t stream_id, const RecvMsg& head) { return 0; }
      void send_goaway(http::errc error_code, const char *const data = nullptr, std::uint32_t data_sz = 0) { this->close((int)error_code); }

      bool send_message_head(std::uint64_t transaction_id, const v1_message_head& head);
      //bool send_message_body(std::uint64_t transaction_id, const char*const data, std::size_t data_sz);
      bool end_message(std::uint32_t transaction_id, const v1_header_block& trailers = v1_header_block());
    private:

      struct queued_send_message
      {
        const std::uint32_t id;
        std::string head_data;
        bool head_sent;
        bool chunked_encoding;
        std::uint64_t content_length;
        std::queue<std::vector<char>> body;
        std::string trailer_data;
        bool ended;
        queued_send_message(std::uint32_t transaction_id)
          : id(transaction_id), head_sent(false), chunked_encoding(false), content_length(0), ended(false)
        {
        }
      };

      struct queued_recv_message
      {
        const std::uint32_t id;
        std::function<void(RecvMsg&& headers)> on_headers;
        std::function<void(RecvMsg&& headers)> on_informational_headers;
        std::function<void(header_block&& headers)> on_trailers;
        std::function<void(const char* const buf, std::size_t buf_size)> on_data;
        std::function<void()> on_end;
        std::function<void()> on_drain;
        queued_recv_message(std::uint32_t transaction_id)
          : id(transaction_id)
        {
        }
      };

      struct queued_close_callback
      {
        const std::uint32_t id;
        std::function<void(std::uint32_t error_code)> on_close;
      };

      socket* socket_;
      bool closed_;
      std::array<char, 8192> recv_buffer_;
      std::deque<queued_send_message> send_queue_;
      std::deque<queued_recv_message> recv_queue_;
      std::deque<queued_close_callback> transaction_close_queue_;
      std::uint32_t next_transaction_id_;
      bool send_loop_running_;


      std::function<void(std::uint32_t error_code)> on_close_;
      std::function<void(std::uint32_t transaction_id)> on_new_stream_;

      void run_recv_loop();
      void recv_chunk_encoded_body();
      void recv_trailers();
      void recv_known_length_body(std::uint64_t content_length);
      void run_send_loop();
    };
  }
}

#endif //MANIFOLD_HTTP_V1_CONNECTION_HPP
