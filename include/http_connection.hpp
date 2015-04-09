#pragma once

#ifndef MANIFOLD_HTTP_CONNECTION_HPP
#define MANIFOLD_HTTP_CONNECTION_HPP

#include <list>
#include <map>
#include <memory>
#include <vector>

#include "asio.hpp"
#include "http_frame.hpp"
#include "http_message_head.hpp"

namespace manifold
{
  namespace http
  {
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
      struct stream
      {
        std::function<void(const char* const buf, std::size_t buf_size)> on_data_frame_;
        std::function<void()> on_end_frame_;
        std::function<void()> on_window_update_;

        std::vector<frame> frames_;
      };
      //================================================================//
    private:
      //----------------------------------------------------------------//
      asio::ip::tcp::socket socket_;
      std::map<setting_code,std::uint32_t> settings_;
      std::array<char, 65535> incoming_buffer_; // For http/2 flow control window.
      //td::set<std::shared_ptr<http::incoming_message>> incomingMessages_;
      //td::set<std::shared_ptr<http::outgoing_message>> outgoingMessages_;
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      // Connection level callbacks:
      std::function<void(std::int32_t stream_id, std::list<std::pair<std::string,std::string>>&& headers, std::int32_t stream_dependency_id)> on_new_stream_;
      std::function<void()> on_close_;
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      std::map<std::int32_t, stream> streams_;
      http::frame incoming_frame_;
      //----------------------------------------------------------------//
    public:
      //----------------------------------------------------------------//
      connection(asio::ip::tcp::socket&& sock);
      ~connection();
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      void run();
      void close();
      void cancelAllStreams();
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      void on_new_stream(const std::function<void(std::int32_t stream_id, std::list<std::pair<std::string,std::string>>&& headers, std::int32_t stream_dependency_id)>& fn);
      void on_close(const std::function<void()>& fn);
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      void on_data_frame(std::uint32_t stream_id, const std::function<void(const char* const buf, std::size_t buf_size)>& fn);
      void on_end_frame(std::uint32_t stream_id, const std::function<void()>& fn);
      void on_rst_stream_frame(std::uint32_t stream_id, std::function<void(const std::error_code& ec)>& fn);
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      void on_window_update(std::uint32_t stream_id, const std::function<void()>& fn);
      bool send_headers_frame(std::uint32_t stream_id, const message_head& head);
      bool send_data_frame(std::uint32_t stream_id, const char*const data, std::size_t data_sz);
      void send_end_frame(std::uint32_t stream_id);
      void send_end_frame(std::uint32_t stream_id, const char*const data, std::size_t data_sz);
      //----------------------------------------------------------------//


      //void send(char* buf, std::size_t buf_size, const std::function<void(const std::error_code& ec, std::size_t bytes_transferred)>& handler);
      //void recv(const char* buf, std::size_t buf_size, const std::function<void(const std::error_code& ec, std::size_t bytes_transferred)>& handler);
    //================================================================//
    };
  }
}



#endif //MANIFOLD_HTTP_CONNECTION_HPP