#pragma once

#ifndef MANIFOLD_HTTP_FRAME_HPP
#define MANIFOLD_HTTP_FRAME_HPP


#include <vector>
#include <list>
#include <cstdint>

#include "socket.hpp"
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
    class frame_header
    {
    public:
      //----------------------------------------------------------------//
      static future<void> recv(manifold::socket& sock, frame_header& destination, std::error_code& ec);
      static future<void> send(manifold::socket& sock, const frame_header& source, std::error_code& ec);
      //----------------------------------------------------------------//
    public:
      //----------------------------------------------------------------//
      frame_header();
      frame_header(std::uint32_t payload_length, frame_type t, std::uint8_t flags, std::uint32_t stream_id);
      frame_header(frame_header&& source);
      virtual ~frame_header();
      frame_header& operator=(frame_header&& source);
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      std::uint32_t payload_length() const;
      frame_type type() const;
      std::uint32_t stream_id() const;
      std::uint8_t flags() const;
      //----------------------------------------------------------------//
    private:
      //----------------------------------------------------------------//
      std::array<char, 9> metadata_;
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      frame_header(const frame_header&) = delete;
      frame_header& operator=(const frame_header&) = delete;
      //----------------------------------------------------------------//
    };
    //================================================================//

    //================================================================//
    class frame_payload : public frame_header
    {
    protected:
      std::vector<char> buf_;
    public:
      frame_payload(std::uint32_t payload_length, frame_type t, std::uint8_t flags, std::uint32_t stream_id) :
        frame_header(payload_length, t, flags, stream_id)
      {}
      frame_payload(frame_payload&& source) :
        frame_header(std::move(source)),
        buf_(std::move(source.buf_))
      {}
      //frame_payload(std::uint8_t flags) : flags_(flags) {}
      virtual ~frame_payload() {}
      frame_payload& operator=(frame_payload&& source)
      {
        if (&source != this)
        {
          frame_header::operator=(std::move(source));
          this->buf_ = std::move(source.buf_);
        }
        return *this;
      }

      std::uint32_t serialized_length() const;


      static future<void> recv(socket& sock, frame_payload& destination, std::error_code& ec);
      static future<void> send(socket& sock, const frame_payload& source, std::error_code& ec);
    };
    //================================================================//

    //================================================================//
    class data_frame : public frame_payload
    {
    public:
      data_frame(std::uint32_t stream_id) : frame_payload(0, frame_type::data, 0, stream_id) {}
      data_frame(std::uint32_t stream_id, const char*const data, std::uint32_t datasz, bool end_stream = false, const char*const padding = nullptr, std::uint8_t paddingsz = 0);
      data_frame(frame_payload&& source) : frame_payload(std::move(source)) {}
      data_frame(data_frame&& source) : frame_payload(std::move(source)) {}
      data_frame& operator=(data_frame&& source)
      {
        frame_payload::operator=(std::move(source));
        return *this;
      }
      ~data_frame() {}

      data_frame split(std::uint32_t num_bytes);

      const char*const data() const;
      std::uint32_t data_length() const;
      const char*const padding() const;
      std::uint8_t pad_length() const;

      bool has_end_stream_flag() const { return this->flags() & frame_flag::end_stream; }
      bool has_padded_flag() const  { return this->flags() & frame_flag::padded; }
    };
    //================================================================//

    //================================================================//
    struct priority_options
    {
      std::uint32_t stream_dependency_id;
      std::uint8_t weight;
      bool exclusive;
      priority_options(std::uint32_t dependency_id, std::uint8_t priority_weight, bool dependency_is_exclusive)
      {
        this->stream_dependency_id = dependency_id;
        this->weight = priority_weight;
        this->exclusive = dependency_is_exclusive;
      }
    };
    //================================================================//

    //================================================================//
    class headers_frame : public frame_payload
    {
    private:
      std::uint8_t bytes_needed_for_pad_length() const;
      std::uint8_t bytes_needed_for_dependency_id_and_exclusive_flag() const;
      std::uint8_t bytes_needed_for_weight() const;
    public:
      headers_frame(std::uint32_t stream_id) : frame_payload(0, frame_type::headers, 0, stream_id) {}
      headers_frame(std::uint32_t stream_id, const char*const header_block, std::uint32_t header_block_sz, bool end_headers, bool end_stream, const char*const padding = nullptr, std::uint8_t paddingsz = 0);
      headers_frame(std::uint32_t stream_id, const char*const header_block, std::uint32_t header_block_sz, bool end_headers, bool end_stream, priority_options priority_ops, const char*const padding = nullptr, std::uint8_t paddingsz = 0);
      headers_frame(frame_payload&& source) : frame_payload(std::move(source)) {}
      headers_frame& operator=(headers_frame&& source)
      {
        frame_payload::operator=(std::move(source));
        return *this;
      }
      ~headers_frame() {}

      const char*const header_block_fragment() const;
      std::uint32_t header_block_fragment_length() const;
      const char*const padding() const;
      std::uint8_t pad_length() const;
      std::uint8_t weight() const;
      std::uint32_t stream_dependency_id() const;
      bool exclusive_stream_dependency() const;

      bool has_end_stream_flag() const { return this->flags() & frame_flag::end_stream; }
      bool has_end_headers_flag() const { return this->flags() & frame_flag::end_headers; }
      bool has_padded_flag() const  { return this->flags() & frame_flag::padded; }
      bool has_priority_flag() const { return this->flags() & frame_flag::priority; }
    };
    //================================================================//

    //================================================================//
    class priority_frame : public frame_payload
    {
    public:
      //priority_frame(frame_payload&& src) : frame_payload(std::move(src)) {}
      priority_frame(std::uint32_t stream_id) : frame_payload(0, frame_type::priority, 0, stream_id) {}
      priority_frame(std::uint32_t stream_id, priority_options options);
      priority_frame(frame_payload&& source) : frame_payload(std::move(source)) {}
      priority_frame& operator=(priority_frame&& source)
      {
        frame_payload::operator=(std::move(source));
        return *this;
      }
      ~priority_frame() {}

      std::uint8_t weight() const;
      std::uint32_t stream_dependency_id() const;
      bool exclusive_stream_dependency() const;
    };
    //================================================================//

    //================================================================//
    class rst_stream_frame : public frame_payload
    {
    public:
      rst_stream_frame(std::uint32_t stream_id) : frame_payload(0, frame_type::rst_stream, 0, stream_id) {}
      rst_stream_frame(std::uint32_t stream_id, http::v2_errc error_code);
      rst_stream_frame(frame_payload&& source) : frame_payload(std::move(source)) {}
      rst_stream_frame& operator=(rst_stream_frame&& source)
      {
        frame_payload::operator=(std::move(source));
        return *this;
      }
      ~rst_stream_frame() {}

      std::uint32_t error_code() const;
    };
    //================================================================//

    class ack_flag { };

    //================================================================//
    class settings_frame : public frame_payload
    {
    public:
      settings_frame() : frame_payload(0, frame_type::settings, 0, 0) {}
      settings_frame(ack_flag) : frame_payload(0, frame_type::settings, 0x1, 0) {}
      settings_frame(std::list<std::pair<std::uint16_t,std::uint32_t>>::const_iterator beg, std::list<std::pair<std::uint16_t,std::uint32_t>>::const_iterator end);
      settings_frame(frame_payload&& source) : frame_payload(std::move(source)) {}
      settings_frame& operator=(settings_frame&& source)
      {
        frame_payload::operator=(std::move(source));
        return *this;
      }
      ~settings_frame() {}


      bool has_ack_flag() const { return (bool)(this->flags() & 0x1); }
      std::list<std::pair<std::uint16_t,std::uint32_t>> settings() const;
    };
    //================================================================//

    //================================================================//
    class push_promise_frame : public frame_payload // TODO: Impl optional padding. Also flags need to looked at!!
    {
    public:
      push_promise_frame(std::uint32_t stream_id) : frame_payload(0, frame_type::push_promise, 0, stream_id) {}
      push_promise_frame(std::uint32_t stream_id, const char*const header_block, std::uint32_t header_block_sz, std::uint32_t promise_stream_id, bool end_headers, const char*const padding = nullptr, std::uint8_t paddingsz = 0);
      push_promise_frame(frame_payload&& source) : frame_payload(std::move(source)) {}
      push_promise_frame& operator=(push_promise_frame&& source)
      {
        frame_payload::operator=(std::move(source));
        return *this;
      }
      ~push_promise_frame() {}

      const char*const header_block_fragment() const;
      std::uint32_t header_block_fragment_length() const;
      const char*const padding() const;
      std::uint8_t pad_length() const;
      std::uint32_t promised_stream_id() const;
      bool has_end_headers_flag() const { return this->flags() & frame_flag::end_headers; }
    };
    //================================================================//

    //================================================================//
    class ping_frame : public frame_payload
    {
    public:
      ping_frame() : frame_payload(0, frame_type::ping, 0, 0) {}
      ping_frame(std::uint64_t ping_data, bool ack = false);
      ping_frame(frame_payload&& source) : frame_payload(std::move(source)) {}
      ping_frame& operator=(ping_frame&& source)
      {
        frame_payload::operator=(std::move(source));
        return *this;
      }
      ~ping_frame() {}

      bool is_ack() const { return (bool)(this->flags() & 0x1); }
      std::uint64_t data() const;
    };
    //================================================================//

    //================================================================//
    class goaway_frame : public frame_payload
    {
    public:
      goaway_frame() : frame_payload(0, frame_type::goaway, 0, 0) {}
      goaway_frame(std::uint32_t last_stream_id, http::v2_errc error_code, const char*const addl_error_data, std::uint32_t addl_error_data_sz);
      goaway_frame(frame_payload&& source) : frame_payload(std::move(source)) {}
      goaway_frame& operator=(goaway_frame&& source)
      {
        frame_payload::operator=(std::move(source));
        return *this;
      }
      ~goaway_frame() {}

      std::uint32_t last_stream_id() const;
      http::v2_errc error_code() const;
      const char*const additional_debug_data() const;
      std::uint32_t additional_debug_data_length() const;
    };
    //================================================================//

    //================================================================//
    class window_update_frame : public frame_payload
    {
    public:
      window_update_frame(std::uint32_t stream_id) : frame_payload(0, frame_type::window_update, 0, stream_id) {}
      window_update_frame(std::uint32_t stream_id, std::uint32_t window_size_increment);
      window_update_frame(frame_payload&& source) : frame_payload(std::move(source)) {}
      window_update_frame& operator=(window_update_frame&& source)
      {
        frame_payload::operator=(std::move(source));
        return *this;
      }
      ~window_update_frame() {}

      std::int32_t window_size_increment() const;
    };
    //================================================================//

    //================================================================//
    class continuation_frame : public frame_payload
    {
    public:
      continuation_frame() : frame_payload(0, frame_type::continuation, 0, 0) {}
      continuation_frame(std::uint32_t stream_id, const char*const header_data, std::uint32_t header_data_sz, bool end_headers);
      continuation_frame(frame_payload&& source) : frame_payload(std::move(source)) {}
      continuation_frame& operator=(continuation_frame&& source)
      {
        frame_payload::operator=(std::move(source));
        return *this;
      }
      ~continuation_frame() {}

      const char*const header_block_fragment() const;
      std::uint32_t header_block_fragment_length() const;

      bool has_end_headers_flag() const { return this->flags() & frame_flag::end_headers; }
    };
    //================================================================//
  }
}

#endif //MANIFOLD_HTTP_FRAME_HPP