#pragma once

#ifndef MANIFOLD_HTTP_FRAME_HPP
#define MANIFOLD_HTTP_FRAME_HPP

#include <vector>
#include <list>
#include <cstdint>

#include "asio.hpp"
#include "http_error_category.hpp"

namespace manifold
{
  namespace http
  {
    //================================================================//
//    enum class errc : std::uint32_t // TODO: Make error_condition
//    {
//      no_error            = 0x0,
//      protocol_error      = 0x1,
//      internal_error      = 0x2,
//      flow_control_error  = 0x3,
//      settings_timeout    = 0x4,
//      stream_closed       = 0x5,
//      frame_size_error    = 0x6,
//      refused_stream      = 0x7,
//      cancel              = 0x8,
//      compression_error   = 0x9,
//      connect_error       = 0xa,
//      enhance_your_calm   = 0xb,
//      inadequate_security = 0xc,
//      http_1_1_required   = 0xd
//    };
    //================================================================//


    //================================================================//
    class frame_flag
    {
    public:
      static const std::uint8_t end_stream  = 0x01;
      static const std::uint8_t end_headers = 0x04;
      static const std::uint8_t padded      = 0x08;
      static const std::uint8_t priority    = 0x20;
    };
    //================================================================//

    //================================================================//
    class frame_payload_base
    {
    protected:
      std::vector<char> buf_;
      std::uint8_t flags_;
    public:
      frame_payload_base(std::uint8_t flags) : flags_(flags) {}
      ~frame_payload_base() {}

      std::uint8_t flags() const;
      std::uint32_t serialized_length() const;
      static void recv_frame_payload(asio::ip::tcp::socket& sock, frame_payload_base& destination, std::uint32_t payload_size, std::uint8_t flags, const std::function<void(const std::error_code& ec)>& cb);
    };
    //================================================================//

    //================================================================//
    class data_frame : public frame_payload_base
    {
    private:
      std::uint8_t bytes_required_for_pad_length() const { return (std::uint8_t)(this->flags_ & frame_flag::padded ? 1 : 0); }
    public:
      data_frame() : frame_payload_base(0) {}
      data_frame(const char*const data, std::uint32_t datasz, std::uint8_t flags);
      ~data_frame() {}

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
      headers_frame() : frame_payload_base(0) {}
      headers_frame(const char*const header_block, std::uint32_t header_block_sz, std::uint8_t weight, std::uint32_t stream_dependency_id, bool exclusive, std::uint8_t flags);
      ~headers_frame() {}

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
      priority_frame() : frame_payload_base(0) {}
      priority_frame(std::uint8_t weight, std::uint32_t stream_dependency_id, bool exclusive, std::uint8_t flags);
      ~priority_frame() {}

      std::uint8_t weight() const;
      std::uint32_t stream_dependency_id() const;
      bool exclusive_stream_dependency() const;
    };
    //================================================================//

    //================================================================//
    class rst_stream_frame : public frame_payload_base
    {
    public:
      rst_stream_frame() : frame_payload_base(0) {}
      rst_stream_frame(http::errc error_code, std::uint8_t flags);
      ~rst_stream_frame() {}

      http::errc error_code() const;
    };
    //================================================================//

    //================================================================//
    class settings_frame : public frame_payload_base
    {
    private:
      std::list<std::pair<std::uint16_t,std::uint32_t>> settings_;
      void serialize_settings();
      void deserialize_settings();
    public:
      settings_frame() : frame_payload_base(0) {}
      settings_frame(std::list<std::pair<std::uint16_t,std::uint32_t>>::const_iterator beg, std::list<std::pair<std::uint16_t,std::uint32_t>>::const_iterator end, std::uint8_t flags);
      ~settings_frame() {}


      std::list<std::pair<std::uint16_t,std::uint32_t>>::const_iterator begin();
      std::list<std::pair<std::uint16_t,std::uint32_t>>::const_iterator end();
      std::size_t count();
    };
    //================================================================//

    //================================================================//
    class push_promise_frame : public frame_payload_base
    {
    public:
      push_promise_frame() : frame_payload_base(0) {}
      push_promise_frame(const char*const header_block, std::uint32_t header_block_sz, std::uint32_t promise_stream_id, std::uint8_t flags);
      ~push_promise_frame() {}

      const char*const header_block_fragment() const;
      std::uint32_t header_block_fragment_length() const;
      const char*const padding() const;
      std::uint8_t pad_length() const;
      std::uint32_t promised_stream_id() const;
    };
    //================================================================//

    //================================================================//
    class ping_frame : public frame_payload_base
    {
    public:
      ping_frame() : frame_payload_base(0) {}
      ping_frame(std::uint64_t ping_data, std::uint8_t flags);
      ~ping_frame() {}

      std::uint64_t data() const;
    };
    //================================================================//

    //================================================================//
    class goaway_frame : public frame_payload_base
    {
    public:
      goaway_frame() : frame_payload_base(0) {}
      goaway_frame(std::uint32_t last_stream_id, std::uint32_t error_code, const char*const addl_error_data, std::uint32_t addl_error_data_sz, std::uint8_t flags);
      ~goaway_frame() {}

      std::uint32_t last_stream_id() const;
      http::errc error_code() const;
      const char*const additional_debug_data() const;
      std::uint32_t additional_debug_data_length() const;
    };
    //================================================================//

    //================================================================//
    class window_update_frame : public frame_payload_base
    {
    public:
      window_update_frame() : frame_payload_base(0) {}
      window_update_frame(std::uint32_t window_size_increment, std::uint8_t flags);
      ~window_update_frame() {}

      std::uint32_t window_size_increment() const;
    };
    //================================================================//

    //================================================================//
    class continuation_frame : public frame_payload_base
    {
    public:
      continuation_frame() : frame_payload_base(0) {}
      continuation_frame(const char*const header_data, std::uint32_t header_data_sz, std::uint8_t flags);
      ~continuation_frame() {}

      const char*const header_block_fragment() const;
      std::uint32_t header_block_fragment_length() const;
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
      goaway,
      window_update,
      continuation,
      invalid_type = 0xFF
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
      const http::goaway_frame        default_goaway_frame_       ;
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
        http::push_promise_frame  push_promise_frame_;
        http::ping_frame          ping_frame_;
        http::goaway_frame        goaway_frame_;
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

      //----------------------------------------------------------------//
      void destroy_union();
      void init_meta(frame_type t, std::uint32_t payload_length, std::uint32_t stream_id, std::uint8_t flags);
      std::uint8_t flags() const;
      //----------------------------------------------------------------//
    public:
      //----------------------------------------------------------------//
      frame();
      frame(http::data_frame&& payload, std::uint32_t stream_id);
      frame(http::headers_frame&& payload, std::uint32_t stream_id);
      frame(http::priority_frame&& payload, std::uint32_t stream_id);
      frame(http::rst_stream_frame&& payload, std::uint32_t stream_id);
      frame(http::settings_frame&& payload, std::uint32_t stream_id);
      frame(http::push_promise_frame&& payload, std::uint32_t stream_id);
      frame(http::ping_frame&& payload, std::uint32_t stream_id);
      frame(http::goaway_frame&& payload, std::uint32_t stream_id);
      frame(http::window_update_frame&& payload, std::uint32_t stream_id);
      frame(http::continuation_frame&& payload, std::uint32_t stream_id);
      frame(frame&& source) {}
      ~frame();
      frame& operator=(frame&& source) { return (*this); }
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      template <typename T>
      bool is() const;

      std::uint32_t payload_length();
      frame_type type() const;
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
      const http::goaway_frame&         goaway_frame()        const;
      const http::window_update_frame&  window_update_frame() const;
      const http::continuation_frame&   continuation_frame()  const;
      //----------------------------------------------------------------//
    };
    //================================================================//
  };
}



#endif //MANIFOLD_HTTP_FRAME_HPP