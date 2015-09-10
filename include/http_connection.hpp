#pragma once
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

#ifndef MANIFOLD_HTTP_CONNECTION_HPP
#define MANIFOLD_HTTP_CONNECTION_HPP

#include <random>
#include <list>
#include <queue>
#include <map>
#include <memory>
#include <vector>
#include <queue>

#include "asio.hpp"
#include "asio/ssl.hpp"
#include "http_frame.hpp"
#include "http_message_head.hpp"

namespace manifold
{
  //================================================================//
  // These wrappers are to add move semantics.
  class socket
  {
  public:
    socket(asio::io_service &ioservice)
      : s_(new asio::ip::tcp::socket(ioservice)) { }

    socket(socket&& source)
      : s_(source.s_)
    {
      source.s_ = nullptr;
    }

    ~socket()
    {
      if (this->s_)
        delete this->s_;
    }
    operator asio::ip::tcp::socket() const { return *this->s_; }
    operator asio::ip::tcp::socket() { return *this->s_; }
  private:
    asio::ip::tcp::socket* s_;
  };
  //================================================================//

  //================================================================//
  class tls_socket
  {
  public:
    tls_socket(asio::io_service &ioservice, asio::ssl::context& ctx)
      : s_(new asio::ssl::stream<asio::ip::tcp::socket>(ioservice, ctx)) { }

    tls_socket(tls_socket&& source)
      : s_(source.s_)
    {
      source.s_ = nullptr;
    }

    ~socket()
    {
      if (this->s_)
        delete this->s_;
    }
    operator asio::ssl::stream<asio::ip::tcp::socket>() const { return *this->s_; }
    operator asio::ssl::stream<asio::ip::tcp::socket>() { return *this->s_; }
  private:
    asio::ssl::stream<asio::ip::tcp::socket>* s_;
  };
  //================================================================//

  namespace http
  {
    //================================================================//
    class connection_io_impl
    {
    public:
      connection_io_impl() {}
      virtual ~connection_io_impl() {}
      virtual void recv_frame(frame& destination, const std::function<void(const std::error_code& ec)>& cb) = 0;
      virtual void send_frame(const frame& source, const std::function<void(const std::error_code& ec)>& cb) = 0;
      virtual bool is_encrypted() const = 0;
      virtual asio::ip::tcp::socket::lowest_layer_type& socket() = 0;
    };
    //================================================================//

    //================================================================//
    class tls_connection_io_impl : public connection_io_impl
    {
    private:
      manifold::tls_socket socket_stream_;
    protected:
      void recv_frame(frame& destination, const std::function<void(const std::error_code& ec)>& cb)
      {
        frame::recv_frame(this->socket_stream_, destination, cb);
      }
      void send_frame(const frame& source, const std::function<void(const std::error_code& ec)>& cb)
      {
        frame::send_frame(this->socket_stream_, source, cb);
      }
    public:
      tls_connection_io_impl(manifold::tls_socket&& sock)
          : socket_stream_(std::move(sock))
      {}
      ~tls_connection_io_impl() {}

      asio::ip::tcp::socket::lowest_layer_type& socket() { return this->ssl_stream().lowest_layer(); }
      asio::ssl::stream<asio::ip::tcp::socket>& ssl_stream() { return this->socket_stream_; }
      bool is_encrypted() const { return true; }
    };
    //================================================================//

    //================================================================//
    class non_tls_connection_io_impl : public connection_io_impl
    {
    private:
      manifold::socket socket_;
    protected:
      void recv_frame(frame& destination, const std::function<void(const std::error_code& ec)>& cb)
      {
        frame::recv_frame(this->socket_, destination, cb);
      }
      void send_frame(const frame& source, const std::function<void(const std::error_code& ec)>& cb)
      {
        frame::send_frame(this->socket_, source, cb);
      }
    public:
      non_tls_connection_io_impl(asio::io_service& ioservice)
        : socket_(ioservice)
      {}
      ~non_tls_connection_io_impl() {}

      asio::ip::tcp::socket::lowest_layer_type& socket() { return this->socket_; }
      asio::ip::tcp::socket& raw_socket() { return this->socket_; }
      bool is_encrypted() const { return false; }
    };
    //================================================================//

    //================================================================//
    class connection : public std::enable_shared_from_this<connection>
    {
    private:
      //================================================================//
      enum class setting_code
      {
        header_table_size      = 0x1, // 4096
        enable_push            = 0x2, // 1
        max_concurrent_streams = 0x3, // (infinite)
        initial_window_size    = 0x4, // 65535
        max_frame_size         = 0x5, // 16384
        max_header_list_size   = 0x6  // (infinite)
      };
      //================================================================//

      //================================================================//
      enum class stream_state
      {
        idle = 0,
        reserved_local,
        reserved_remote,
        open,
        half_close_local,
        half_closed_remote,
        closed
      };
      struct stream
      {
        const std::uint32_t id;
        stream_state state = stream_state::idle;
        std::function<void(const char* const buf, std::size_t buf_size)> on_data;
        std::function<void(http::header_block&& headers)> on_headers;
        std::function<void(std::uint32_t error_code)> on_rst_stream;
        std::function<void(http::header_block&& headers, std::uint32_t promised_stream_id)> on_push_promise;
        std::function<void()> on_end;
        std::function<void()> on_drain;

        std::function<void(std::uint32_t error_code)> on_close;

        std::queue<header_block> incoming_message_heads;
        std::queue<frame> incoming_data_frames;
        std::queue<frame> outgoing_non_data_frames;
        std::queue<frame> outgoing_data_frames;

        std::uint32_t outgoing_window_size = 65535;
        std::uint32_t incoming_window_size = 65535;
        std::uint32_t stream_dependency_id = 0;
        std::uint8_t weight = 16;

        bool end_stream_frame_received = false;
        stream(std::uint32_t stream_id) : id(stream_id) {}
      };
      //================================================================//

      //================================================================//
      class stream_dependency_tree
      {
      private:
        //----------------------------------------------------------------//
        std::vector<stream_dependency_tree> children_;
        stream* stream_ptr_;
        //----------------------------------------------------------------//
      public:
        //----------------------------------------------------------------//
        stream_dependency_tree(stream* stream_ptr);
        stream_dependency_tree(stream* stream_ptr, const std::vector<stream_dependency_tree>& children);
        ~stream_dependency_tree() {}
        //----------------------------------------------------------------//


        //----------------------------------------------------------------//
        stream* stream_ptr() const;
        const std::vector<stream_dependency_tree>& children() const;
        void insert_child(stream_dependency_tree&& child);
        //----------------------------------------------------------------//
      };
      //================================================================//
    private:
      //----------------------------------------------------------------//
      connection_io_impl* io_impl_;
      std::map<setting_code,std::uint32_t> settings_;
      hpack::encoder hpack_encoder_;
      hpack::decoder hpack_decoder_;
      bool started_;
      bool send_loop_running_;
      std::minstd_rand rg_;
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      // Connection level callbacks:
      std::function<void(std::uint32_t stream_id, header_block&& headers)> on_new_stream_;
      std::function<void(std::uint32_t error_code)> on_close_;
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      std::uint32_t last_newly_accepted_stream_id_;
      std::uint32_t last_newly_created_stream_id_;
      std::map<std::uint32_t,stream> streams_;
      stream root_stream_; // used for connection level frames
      http::frame incoming_frame_;
      std::queue<http::frame> incoming_header_block_fragments_;
      http::frame outgoing_frame_;
      stream_dependency_tree stream_dependency_tree_;
      std::uint32_t connection_level_outgoing_window_size() const { return this->root_stream_.outgoing_window_size; }
      std::uint32_t connection_level_incoming_window_size() const { return this->root_stream_.incoming_window_size; }
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      stream* get_next_send_stream_ptr(const stream_dependency_tree& current_node);
      bool check_tree_for_outgoing_frame(const stream_dependency_tree& current_node);
      bool stream_has_sendable_frame(const stream& stream_to_check);
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      void send_connection_level_window_update(std::uint32_t amount);
      void send_ping_acknowledgement(std::uint64_t opaque_data);
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      void run_recv_loop();
      void run_send_loop();
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      void handle_incoming_frame(stream& stream, const data_frame& incoming_data_frame);
      void handle_incoming_frame(stream& stream, const headers_frame& incoming_headers_frame, const std::vector<continuation_frame>& continuation_frames);
      void handle_incoming_frame(stream& stream, const priority_frame& incoming_priority_frame);
      void handle_incoming_frame(stream& stream, const rst_stream_frame& incoming_rst_stream_frame);
      void handle_incoming_frame(const settings_frame& incoming_settings_frame);
      void handle_incoming_frame(stream& stream, const push_promise_frame& incoming_push_promise_frame, const std::vector<continuation_frame>& continuation_frames);
      void handle_incoming_frame(const ping_frame& incoming_ping_frame);
      void handle_incoming_frame(const goaway_frame& incoming_goaway_frame);
      void handle_incoming_frame(const window_update_frame& incoming_window_update_frame);
      void handle_incoming_frame(stream& stream, const window_update_frame& incoming_window_update_frame);
      //void handle_incoming_frame(stream& frame_stream, const continuation_frame&  incoming_frame);
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      bool handle_outgoing_headers_state_change(stream& stream);
      bool handle_outgoing_end_stream_state_change(stream& stream);
      bool handle_outgoing_rst_stream_state_change(stream& stream);
      //----------------------------------------------------------------//
    public:
      //----------------------------------------------------------------//
      connection(connection_io_impl* io_impl);
      virtual ~connection();
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      static const std::array<char,24> preface;
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      void run();
      void close();
      void cancelAllStreams();
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      void on_new_stream(const std::function<void(std::int32_t stream_id, header_block&& headers)>& fn);
      void on_close(const std::function<void(std::uint32_t error_code)>& fn);
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      // connection-only frames: settings, ping, goaway
      // window_update is for both.
      void on_data(std::uint32_t stream_id, const std::function<void(const char* const buf, std::size_t buf_size)>& fn);
      void on_headers(std::uint32_t stream_id, const std::function<void(http::header_block&& headers)>& fn);
      void on_close(std::uint32_t stream_id, const std::function<void(std::uint32_t error_code)>& fn);
      void on_push_promise(std::uint32_t stream_id, const std::function<void(http::header_block&& headers, std::uint32_t promised_stream_id)>& fn);


      void on_end(std::uint32_t stream_id, const std::function<void()>& fn);
      //void on_window_update(std::uint32_t stream_id, const std::function<void()>& fn);
      void on_drain(std::uint32_t stream_id, const std::function<void()>& fn);
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      bool create_stream(std::uint32_t stream_id);
      bool send_data(std::uint32_t stream_id, const char *const data, std::uint32_t data_sz, bool end_stream);
      bool send_headers(std::uint32_t stream_id, const header_block& head, bool end_headers, bool end_stream);
      bool send_headers(std::uint32_t stream_id, const header_block& head, priority_options priority, bool end_headers, bool end_stream);
      bool send_priority(std::uint32_t stream_id, priority_options options);
      bool send_reset_stream(std::uint32_t stream_id, http::errc error_code);
      void send_settings(const std::list<std::pair<std::uint16_t,std::uint32_t>>& settings);
      bool send_push_promise(std::uint32_t stream_id, const header_block& head, std::uint32_t promised_stream_id, bool end_headers);
      void send_ping(std::uint64_t opaque_data);
      void send_goaway(http::errc error_code, const char *const data = nullptr, std::uint32_t data_sz = 0);
      bool send_window_update(std::uint32_t stream_id, std::uint32_t amount);
      bool send_countinuation(std::uint32_t stream_id, const header_block& head, bool end_headers);
      //----------------------------------------------------------------//


      //void send(char* buf, std::size_t buf_size, const std::function<void(const std::error_code& ec, std::size_t bytes_transferred)>& handler);
      //void recv(const char* buf, std::size_t buf_size, const std::function<void(const std::error_code& ec, std::size_t bytes_transferred)>& handler);
    };
    //================================================================//
  }
}



#endif //MANIFOLD_HTTP_CONNECTION_HPP