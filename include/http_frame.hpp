#pragma once

#ifndef IPSUITE_HTTP_FRAME_HPP
#define IPSUITE_HTTP_FRAME_HPP

#include <vector>
#include <cstdint>

#include "asio.hpp"

namespace manifold
{
  namespace http
  {
    //================================================================//
    class frame_payload_base
    {
    protected:
      std::vector<char> buf_;
    public:
      static void recv_frame_payload(asio::ip::tcp::socket& sock, frame_payload_base& destination, std::uint32_t payload_size, const std::function<void(const std::error_code& ec)>& cb);
    };
    //================================================================//

    //================================================================//
    class data_frame : public frame_payload_base
    {
    public:
      const char*const data() const;
      std::uint32_t data_length() const;
      const char*const padding() const;
      std::uint8_t pad_length() const;
    };
    //================================================================//

    //================================================================//
    class headers_frame : public frame_payload_base
    {
    public:
      const char*const header_block_fragment() const;
      std::uint32_t header_block_fragment_length() const;
      const char*const padding() const;
      std::uint8_t pad_length() const;
      std::uint8_t weight() const;
      std::uint32_t stream_dependency_id() const;
      bool exclusive_stream_dependency() const;
    };
    //================================================================//

    //================================================================//
    class priority_frame : public frame_payload_base
    {
    public:
      std::uint8_t weight() const;
      std::uint32_t stream_dependency_id() const;
      bool exclusive_stream_dependency() const;
    };
    //================================================================//

    //================================================================//
    class rst_stream_frame : public frame_payload_base
    {
    public:
      std::uint32_t error_code() const; // TODO: Make enum.
    };
    //================================================================//

    //================================================================//
    class settings_frame : public frame_payload_base
    {
    private:
      std::vector<std::pair<std::uint16_t,std::uint32_t>> settings_;
      void deserialize_settings();
    public:
      std::vector<std::pair<std::uint16_t,std::uint32_t>>::const_iterator begin();
      std::vector<std::pair<std::uint16_t,std::uint32_t>>::const_iterator end();
      std::size_t count();
    };
    //================================================================//

    //================================================================//
    class push_promise_frame : public frame_payload_base
    {
    public:
    };
    //================================================================//

    //================================================================//
    class ping_frame : public frame_payload_base
    {
    public:
    };
    //================================================================//

    //================================================================//
    class go_away_frame : public frame_payload_base
    {
    public:
    };
    //================================================================//

    //================================================================//
    class window_update_frame : public frame_payload_base
    {
    public:
    };
    //================================================================//

    //================================================================//
    class continuation_frame : public frame_payload_base
    {
    public:
    };
    //================================================================//

    //================================================================//
    enum class frame_type : std::uint8_t
    {
      data = 0x0,
      headers,
      priority,
      rst_stream,
      settings,
      push_promise,
      ping,
      go_away,
      window_update,
      continuation
    };
    //================================================================//

    //================================================================//
    class frame
    {
    public:
      //----------------------------------------------------------------//
      const http::data_frame          default_data_frame_         ;
      const http::headers_frame       default_headers_frame_      ;
      const http::priority_frame      default_priority_frame_     ;
      const http::rst_stream_frame    default_rst_stream_frame_   ;
      const http::settings_frame      default_settings_frame_     ;
      const http::push_promise_frame  default_push_promise_frame_ ;
      const http::ping_frame          default_ping_frame_         ;
      const http::go_away_frame       default_go_away_frame_      ;
      const http::window_update_frame default_window_update_frame_;
      const http::continuation_frame  default_continuation_frame_ ;
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      static void recv_frame(asio::ip::tcp::socket& sock, frame& destination, const std::function<void(const std::error_code& ec)>& cb);
      static void send_frame(asio::ip::tcp::socket& sock, const frame& source, const std::function<void(const std::error_code& ec)>& cb);
      //----------------------------------------------------------------//
    private:
      //----------------------------------------------------------------//
      union payload_union
      {
        //----------------------------------------------------------------//
        http::data_frame          data_frame_;
        http::headers_frame       headers_frame_;
        http::priority_frame      priority_frame_;
        http::rst_stream_frame    rst_stream_frame_;
        http::settings_frame      settings_frame_;
        http::push_promise_frame  push_promise_frame_
        http::ping_frame          ping_frame_;
        http::go_away_frame       go_away_frame_;
        http::window_update_frame window_update_frame_;
        http::continuation_frame  continuation_frame_;
        //----------------------------------------------------------------//

        //----------------------------------------------------------------//
        payload_union(){}
        ~payload_union(){}
        //----------------------------------------------------------------//
      };
      //----------------------------------------------------------------//
    private:
      //----------------------------------------------------------------//
      payload_union payload_;
      std::array<char, 9> metadata_;
      //----------------------------------------------------------------//
    public:
//      frame(frame_type t, std::uint8_t flags, std::vector<char>&& payload, std::int32_t stream_id = 0)
//        : type_(t), flags_(flags), payload_(std::move(payload)), stream_id_(stream_id) {}
//      ~frame() {}

      //----------------------------------------------------------------//
      template <typename T>
      bool is() const;

      std::uint32_t payload_length();
      frame_type type() const;
      std::uint8_t flags() const;
      std::uint32_t stream_id() const;
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      const http::data_frame&           data_frame()          const;
      const http::headers_frame&        headers_frame()       const;
      const http::priority_frame&       priority_frame()      const;
      const http::rst_stream_frame&     rst_stream_frame()    const;
      const http::settings_frame&       settings_frame()      const;
      const http::push_promise_frame&   push_promise_frame()  const;
      const http::ping_frame&           ping_frame()          const;
      const http::go_away_frame&        go_away_frame()       const;
      const http::window_update_frame&  window_update_frame() const;
      const http::continuation_frame&   continuation_frame()  const;
      //----------------------------------------------------------------//
    };
    //================================================================//
  };
}



#endif //IPSUITE_HTTP_FRAME_HPP