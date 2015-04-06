#pragma once

#ifndef IPSUITE_HTTP_CONNECTION_HPP
#define IPSUITE_HTTP_CONNECTION_HPP

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

      //================================================================//

      //================================================================//
      class frame
      {
      public:
        enum class type { data = 0x0, headers, priority, rst_stream, settings, push_promise, ping, go_away, window_update, continuation };
        union payload_union
        {
          //----------------------------------------------------------------//

          //----------------------------------------------------------------//

          //----------------------------------------------------------------//
          payload_union(){}
          payload_union(const payload_union&){}
          payload_union& operator=(const payload_union&) { return (*this); }
          ~payload_union(){}
          //----------------------------------------------------------------//
        } payload_;
      private:
        type type_;
        std::uint8_t flags_;
        std::vector<char> payload_;
        std::int32_t stream_id_;
      public:
        frame(type t, std::uint8_t flags, std::vector<char>&& payload, std::int32_t stream_id = 0)
          : type_(t), flags_(flags), payload_(std::move(payload)), stream_id_(stream_id) {}
        ~frame() {}
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
      //----------------------------------------------------------------//
    public:
      //----------------------------------------------------------------//
      connection(asio::ip::tcp::socket&& sock);
      ~connection();
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      void on_new_stream(const std::function<void(std::int32_t stream_id, std::list<std::pair<std::string,std::string>>&& headers, std::int32_t stream_dependency_id)>& fn);
      void on_close(const std::function<void()>& fn);
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      void on_data_frame(std::int32_t stream_id, const std::function<void(const char* const buf, std::size_t buf_size)>& fn);
      void on_end_frame(std::int32_t stream_id, const std::function<void()>& fn);
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      void on_window_update(std::int32_t stream_id, const std::function<void()>& fn);
      bool send_headers_frame(std::int32_t stream_id, const message_head& head);
      bool send_data_frame(std::int32_t stream_id, const char*const data, std::size_t data_sz);
      void send_end_frame(std::int32_t stream_id);
      void send_end_frame(std::int32_t stream_id, const char*const data, std::size_t data_sz);
      //----------------------------------------------------------------//


      //void send(char* buf, std::size_t buf_size, const std::function<void(const std::error_code& ec, std::size_t bytes_transferred)>& handler);
      //void recv(const char* buf, std::size_t buf_size, const std::function<void(const std::error_code& ec, std::size_t bytes_transferred)>& handler);
    //================================================================//
    };
  }
}



#endif //IPSUITE_HTTP_CONNECTION_HPP