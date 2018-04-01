#pragma once
#ifndef MANIFOLD_HTTP_CONNECTION_HPP
#define MANIFOLD_HTTP_CONNECTION_HPP

#include "future.hpp"
#include "socket.hpp"
#include "http_frame.hpp"
#include "http_error_category.hpp"
#include "hpack.hpp"
#include "http_header_block.hpp"

#include <asio/spawn.hpp>

#include <memory>
#include <queue>
#include <list>
#include <random>
#include <experimental/coroutine>

#ifdef MANIFOLD_DISABLE_HTTP2
#define MANIFOLD_HTTP_ALPN_SUPPORTED_PROTOCOLS "\x08http/1.1"
#else
#define MANIFOLD_HTTP_ALPN_SUPPORTED_PROTOCOLS "\x2h2\x08http/1.1"
#endif

namespace manifold
{
  namespace http
  {
    //================================================================//
    enum class version
    {
      unknown = 0,
      http1_1,
      http2
    };
    //================================================================//

    //================================================================//
    class connection
    {
    public:
      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
      static const std::uint32_t default_header_table_size      ; //= 4096;
      static const std::uint32_t default_enable_push            ; //= 1;
      static const std::uint32_t default_initial_window_size    ; //= 65535;
      static const std::uint32_t default_max_frame_size         ; //= 16384;
      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
      enum class setting_code
      {
        header_table_size      = 0x1, // 4096
        enable_push            = 0x2, // 1
        max_concurrent_streams = 0x3, // (infinite)
        initial_window_size    = 0x4, // 65535
        max_frame_size         = 0x5, // 16384
        max_header_list_size   = 0x6  // (infinite)
      };
      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
      enum class stream_state
      {
        idle = 0,
        reserved_local,
        reserved_remote,
        open,
        half_closed_local,
        half_closed_remote,
        closed
      };
      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
      class stream : public std::enable_shared_from_this<stream>
      {
      public:
        struct header_awaiter;

        stream_state state() const { return this->state_; }
        std::uint32_t id() const { return this->id_; }

        bool has_something_to_send(bool exclude_data) const;

        bool has_sendable_reset() const;
        v2_errc sendable_reset() const;
        void pop_sendable_reset();

        bool has_sendable_headers() const;
        const header_block& sendable_headers() const;
        void pop_sendable_headers();

        bool has_sendable_data() const;
        std::tuple<const char*, std::size_t> sendable_data();
        void pop_sendable_data(std::uint32_t amount);
        std::size_t out_data_size() const;

        bool has_sendable_window_update() const;
        std::uint32_t sendable_window_update();
        void pop_sendable_window_update();

        bool has_receivable_data() const;


        bool adjust_local_window_size(std::int32_t amount);
        bool adjust_peer_window_size(std::int32_t amount);

        future<header_block> recv_headers();
        future<std::size_t> recv_data(char* dest, std::size_t sz);

        future<bool> send_headers(const header_block& headers, bool end_stream);
        future<std::size_t> send_data(const char* data, std::size_t sz, bool end_stream);

        bool send_reset(v2_errc ec);
        bool send_window_update(std::int32_t amount);

        v2_errc handle_incoming_data(const char*const data, std::size_t sz, std::int32_t local_initial_window_size, bool end_stream);
        v2_errc handle_incoming_headers(header_block&& headers, bool end_stream);
        v2_errc handle_incoming_priority(std::uint8_t weight, std::uint32_t dependency_id);
        v2_errc handle_incoming_rst_stream(std::uint32_t error_code);
        v2_errc handle_incoming_push_promise(header_block&& headers, stream& idle_promised_stream);
        v2_errc handle_incoming_window_update(std::int32_t window_size_increment);

        stream(connection* parent, std::uint32_t stream_id, uint32_t initial_window_size, uint32_t initial_peer_window_size);

      private:
        const std::uint32_t id_;
        stream_state state_ = stream_state::idle;
        connection* parent_connection_;
        bool on_headers_called_ = false;

        std::queue<std::string> in_data_overflow_;
        asio::mutable_buffer in_data_buf_;
        std::queue<header_block> in_headers_;

        bool out_end_ = false;
        const char* out_data_ = nullptr;
        std::size_t out_data_sz_ = 0;
        std::queue<header_block> out_headers_;
        bool out_headers_end_stream_ = false;
        std::uint32_t out_window_update_ = 0;
        std::unique_ptr<v2_errc> out_reset_;

        std::int32_t incoming_window_size_ = 65535;
        std::int32_t outgoing_window_size_ = 65535;
        std::uint32_t stream_dependency_id_ = 0;
        std::uint8_t weight_ = 16;
        bool end_stream_received_ = false;

        std::shared_ptr<future<header_block>::promise_type> recv_headers_promise_;
        std::shared_ptr<future<std::size_t>::promise_type> recv_data_promise_;
        std::shared_ptr<future<bool>::promise_type> send_headers_promise_;
        std::shared_ptr<future<std::size_t>::promise_type> send_data_promise_;

      private:
        //static bool incoming_header_is_informational(const RecvMsg &head);
      };
      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

      static const std::array<char,24> http2_preface;
      static const std::uint32_t max_stream_id = 0x7FFFFFFF;
      static const std::uint32_t initial_stream_id;

      connection(manifold::tls_socket&& sock, http::version http_version, std::function<future<void>(std::shared_ptr<connection::stream>)> on_new_stream_handler = nullptr);
      connection(manifold::non_tls_socket&& sock, http::version http_version, std::function<future<void>(std::shared_ptr<connection::stream>)> on_new_stream_handler = nullptr);
      connection(const connection&) = delete;
      connection(connection&&) = delete;

      http::version version();

      future<void> run_v2_recv_loop();
      future<void> run_v2_send_loop();
      void send_connection_level_window_update(std::int32_t amount);

      std::unordered_map<std::uint32_t, std::shared_ptr<stream>>::iterator find_sendable_stream(bool exclude_data);

//      stream::recv_headers_awaiter recv_headers(std::uint32_t stream_id);
//      stream::recv_data_awaiter recv_data(std::uint32_t stream_id, char* dest, std::size_t sz);
//
//      stream::send_headers_awaiter send_headers(std::uint32_t stream_id, const header_block& headers, bool end_stream);
//      stream::send_data_awaiter send_data(std::uint32_t stream_id, const char* data, std::size_t sz, bool end_stream);
//
//      void send_reset(std::uint32_t stream_id, v2_errc ec);

      std::uint32_t create_client_stream();

      void close(v2_errc ec);
    private:
      std::minstd_rand rng_;
      hpack::decoder hpack_decoder_;
      hpack::encoder hpack_encoder_;
      std::unordered_map<std::uint32_t, std::shared_ptr<stream>> streams_;
      std::map<setting_code,std::uint32_t> peer_settings_;
      std::map<setting_code,std::uint32_t> local_settings_;
      std::queue<std::list<std::pair<std::uint16_t,std::uint32_t>>> pending_local_settings_;
      std::queue<frame_payload> outgoing_frames_;
//      std::queue<settings_frame> outgoing_settings_frames_;
//      std::queue<ping_frame> outgoing_ping_frames_;

      std::function<future<void>(std::shared_ptr<stream> stream_ptr)> on_new_stream_;

      std::unique_ptr<socket> socket_;
      std::uint32_t last_newly_accepted_stream_id_;
      std::int32_t outgoing_window_size_;
      std::int32_t incoming_window_size_;
      http::version protocol_version_;
      bool send_loop_running_;
      bool closed_;
      bool is_server_;
    private:
      connection& operator=(const connection&) = delete;
      connection& operator=(connection&&) = delete;

      v2_errc handle_incoming(settings_frame&& incoming_frame);
      v2_errc handle_incoming(ping_frame&& incoming_frame);
      v2_errc handle_incoming(goaway_frame&& incoming_frame);
      v2_errc handle_incoming(window_update_frame&& incoming_frame);

      void spawn_v2_send_loop_if_needed();

      void post_send_loop_if_needed();
      std::uint32_t gen_stream_id();
    };
    //================================================================//
  }
}

#endif // MANIFOLD_HTTP_CONNECTION_HPP