
#include "manifold/http_connection.hpp"
#include "manifold/http_frame.hpp"

#include <functional>
#include <iostream>

#include <manifold/http_request_head.hpp>

namespace manifold
{
  namespace http
  {
    //================================================================//
    connection::stream::recv_headers_awaiter::recv_headers_awaiter(const std::shared_ptr<connection::stream>& p) :
      parent_stream_(p)
    { }

    bool connection::stream::recv_headers_awaiter::await_ready()
    {
      return !parent_stream_ || parent_stream_->in_headers_.size();
    }

    header_block connection::stream::recv_headers_awaiter::await_resume()
    {
      header_block ret;
      if (parent_stream_ && parent_stream_->in_headers_.size())
      {
        ret = std::move(parent_stream_->in_headers_.front());
        parent_stream_->in_headers_.pop();
      }
      return ret;
    }

    void connection::stream::recv_headers_awaiter::await_suspend(std::experimental::coroutine_handle<> coro)
    {
      if (parent_stream_)
        parent_stream_->recv_headers_coro_ = coro;
    }
    //================================================================//

    //================================================================//
    connection::stream::recv_data_awaiter::recv_data_awaiter(const std::shared_ptr<connection::stream>& p, char* dest, std::size_t sz) :
    parent_stream_(p),
    dest_(dest),
    dest_end_(dest + sz)
    { }

    bool connection::stream::recv_data_awaiter::await_ready()
    {
      return !parent_stream_ || parent_stream_->in_data_.size() || dest_ >= dest_end_;
    }

    std::size_t connection::stream::recv_data_awaiter::await_resume()
    {
      std::size_t ret = 0;

      while (parent_stream_ && parent_stream_->in_data_.size() && dest_ < dest_end_)
      {
        std::size_t front_size = parent_stream_->in_data_.front().size();

        std::size_t remaining_in_dest = dest_end_ - dest_;
        if (front_size > remaining_in_dest)
        {
          std::memcpy(dest_,  parent_stream_->in_data_.front().data(), remaining_in_dest);
          parent_stream_->in_data_.front().erase(0, remaining_in_dest);
          dest_ += remaining_in_dest;
          ret += remaining_in_dest;
        }
        else
        {
          std::memcpy(dest_, parent_stream_->in_data_.front().data(), front_size);
          dest_ += front_size;
          ret += front_size;
          parent_stream_->in_data_.pop();
        }

        parent_stream_->out_window_update_ += std::uint32_t(ret);
      }

      return ret;
    }

    void connection::stream::recv_data_awaiter::await_suspend(std::experimental::coroutine_handle<> coro)
    {
      if (parent_stream_)
        parent_stream_->recv_data_coro_ = coro;
    }
    //================================================================//

    //================================================================//
    connection::stream::send_headers_awaiter::send_headers_awaiter(const std::shared_ptr<connection::stream>& p) :
      parent_stream_(p)
    { }

    bool connection::stream::send_headers_awaiter::await_ready()
    {
      return !parent_stream_;
    }

    void connection::stream::send_headers_awaiter::await_resume()
    {

    }

    void connection::stream::send_headers_awaiter::await_suspend(std::experimental::coroutine_handle<> coro)
    {
      if (parent_stream_)
      {
        parent_stream_->send_headers_coro_ = coro;
      }
    }
    //================================================================//

    //================================================================//
    connection::stream::send_data_awaiter::send_data_awaiter(const std::shared_ptr<connection::stream>& p, std::size_t sz) :
      parent_stream_(p),
      sz_(sz)
    { }

    bool connection::stream::send_data_awaiter::await_ready()
    {
      return !parent_stream_; // TODO: check window.
    }

    std::size_t connection::stream::send_data_awaiter::await_resume()
    {
      std::size_t ret = 0;

      if (parent_stream_)
      {
        return sz_;
      }

      return ret;
    }

    void connection::stream::send_data_awaiter::await_suspend(std::experimental::coroutine_handle<> coro)
    {
      if (parent_stream_)
      {
        parent_stream_->send_data_coro_ = coro;
      }
    }
    //================================================================//

    //================================================================//
    connection::stream::stream(connection* parent_conn, std::uint32_t stream_id, uint32_t initial_window_size, uint32_t initial_peer_window_size) :
      id_(stream_id),
      parent_connection_(parent_conn),
      incoming_window_size_(initial_window_size),
      outgoing_window_size_(initial_peer_window_size)
    {

    }

    bool connection::stream::has_something_to_send(bool exclude_data) const
    {
      return (has_sendable_headers() || (!exclude_data && has_sendable_data()) || has_sendable_window_update());
    }

    bool connection::stream::has_sendable_headers() const
    {
      return !out_headers_.empty();
    }

    const header_block& connection::stream::sendable_headers()
    {
      return out_headers_.front();
    }

    void connection::stream::pop_sendable_headers()
    {
      if (out_headers_.size())
      {
        out_headers_.pop();
        std::experimental::coroutine_handle<> coro;
        std::swap(send_headers_coro_, coro);
        coro ? coro() : void();
      }
    }

    bool connection::stream::has_sendable_data() const
    {
      return out_data_ != nullptr;
    }

    std::tuple<const char*, std::size_t> connection::stream::sendable_data()
    {
      std::size_t amount = out_data_sz_;
      if (amount > outgoing_window_size_)
        amount = outgoing_window_size_;
      return std::make_tuple(out_data_, amount);
    }

    void connection::stream::pop_sendable_data(std::uint32_t amount)
    {
      assert(amount <= outgoing_window_size_);
      outgoing_window_size_ -= amount;

      if (amount < out_data_sz_)
      {
        out_data_ += amount;
        out_data_sz_ -= amount;
      }
      else
      {
        out_data_ = nullptr;
        out_data_sz_ = 0;

        std::experimental::coroutine_handle<> coro;
        std::swap(send_data_coro_, coro);
        coro ? coro() : void();
      }
    }

    std::size_t connection::stream::out_data_size() const
    {
      return out_data_sz_;
    }

    bool connection::stream::has_sendable_window_update() const
    {
      return out_window_update_ > 0;
    }

    std::uint32_t connection::stream::sendable_window_update()
    {
      return out_window_update_;
    }

    void connection::stream::pop_sendable_window_update()
    {
      out_window_update_ = 0;
    }

    bool connection::stream::has_receivable_data() const
    {
      return !in_data_.empty();
    }

    connection::stream::recv_headers_awaiter connection::stream::recv_headers()
    {
      return recv_headers_awaiter{shared_from_this()};
    }

    connection::stream::recv_data_awaiter connection::stream::recv_data(char* dest, std::size_t sz)
    {
      return recv_data_awaiter{shared_from_this(), dest, sz};
    }

    connection::stream::send_headers_awaiter connection::stream::send_headers(const header_block& headers, bool end_stream)
    {
      switch (this->state_)
      {
      case stream_state::idle:
      case stream_state::reserved_local:
      case stream_state::open:
      case stream_state::half_closed_remote:
      {
//        std::string header_data;
//        v2_header_block::serialize(enc, headers, header_data);
//
//        const std::uint8_t EXTRA_BYTE_LENGTH_NEEDED_FOR_HEADERS_FRAME = 0; //TODO: Set correct value
//        if ((header_data.size() + EXTRA_BYTE_LENGTH_NEEDED_FOR_HEADERS_FRAME) > max_frame_size)
//        {
//          assert("impl");
//          // TODO: Split header into continuation frames.
//        }
//        else
        {
          out_headers_.emplace(headers);
          if (end_stream)
            out_end_ = true;
        }

        switch (this->state_)
        {
        case stream_state::idle:
          this->state_ = stream_state::open;
          break;
        case stream_state::reserved_local:
          this->state_ = stream_state::half_closed_remote;
          break;
        case stream_state::open:
          if (end_stream)
            this->state_ = stream_state::half_closed_local;
          break;
        case stream_state::half_closed_remote:
          if (end_stream)
          {
            this->state_ = stream_state::closed;
            this->parent_connection_ = nullptr;
            //this->on_close_ ? this->on_close_(std::error_code()) : void();
          }
          break;
        default:
          break;
        }

        parent_connection_->spawn_v2_send_loop_if_needed();
        return send_headers_awaiter{shared_from_this()};
      }
      case stream_state::reserved_remote:
      case stream_state::half_closed_local:
      case stream_state::closed:
        return send_headers_awaiter{nullptr};
      }

      return send_headers_awaiter{nullptr};
    }

    connection::stream::send_data_awaiter connection::stream::send_data(const char* data, std::size_t sz, bool end_stream)
    {
      // TODO: Check max_frame_size
      switch (this->state_)
      {
      case stream_state::open:
        out_data_  = data;
        out_data_sz_ = sz;

        if (end_stream)
          out_end_ = true;
        if (end_stream)
          this->state_ = stream_state::half_closed_local;
        parent_connection_->spawn_v2_send_loop_if_needed();
        return send_data_awaiter{shared_from_this(), sz};
      case stream_state::half_closed_remote:
        out_data_  = data;
        out_data_sz_ = sz;

        if (end_stream)
          out_end_ = true;
        if (end_stream)
        {
          this->state_ = stream_state::closed;
          //this->on_close_ ? this->on_close_(v2_errc::no_error) : void();
        }
        parent_connection_->spawn_v2_send_loop_if_needed();
        return send_data_awaiter{shared_from_this(), sz};
      case stream_state::reserved_remote:
      case stream_state::idle:
      case stream_state::reserved_local:
      case stream_state::half_closed_local:
      case stream_state::closed:
        return send_data_awaiter{nullptr, 0};
      }

      return send_data_awaiter{nullptr, 0};
    }

    void connection::stream::send_reset(v2_errc ec)
    {
      // TODO: !!!
//      switch (this->state_)
//      {
//      case stream_state::idle:
//      case stream_state::closed:
//        return false;
//      default:
//        this->outgoing_non_data_frames_.push(http::frame(http::rst_stream_frame(ec), this->id_));
//        std::queue<data_frame> rmv;
//        this->outgoing_data_frames_.swap(rmv);
//        this->state_ = stream_state::closed;
//        this->on_close_ ? this->on_close_(ec) : void();
//        return true;
//      }
    }


    v2_errc connection::stream::handle_incoming_data(const char* const data, std::size_t sz, std::int32_t local_initial_window_size, bool end_stream)
    {
      v2_errc ret = v2_errc::no_error;
      switch (this->state_)
      {
        case stream_state::half_closed_local:
        case stream_state::open:
        {
          if (sz > this->incoming_window_size_)
          {
            this->send_reset(v2_errc::flow_control_error);
          }
          else
          {
  //            this->incoming_window_size_ -= sz;
  //            if (this->incoming_window_size_ < local_initial_window_size / 2)
  //              this->out_window_updates_.emplace(local_initial_window_size - this->incoming_window_size_);

            this->in_data_.emplace(data, sz);

            if (end_stream)
            {
              if (this->state_ == stream_state::open)
                this->state_ = stream_state::half_closed_remote;
              else
                this->state_ = stream_state::closed;
            }

            std::experimental::coroutine_handle<> coro{nullptr};
            std::swap(coro, recv_data_coro_);
            coro ? coro() : void();
          }
          break;
        }
        default:
        {

          // TODO: deal with frame / state_ mismatch
        }
      }

      return ret;
    }

    v2_errc connection::stream::handle_incoming_headers(header_block&& headers, bool end_stream)
    {
      v2_errc ret = v2_errc::no_error;
      switch (this->state_)
      {
        case stream_state::idle:
        case stream_state::open:
        case stream_state::half_closed_local:
        case stream_state::reserved_remote:
        {


//          std::string serialized_header_block;
//          std::size_t serialized_header_block_sz = incoming_headers_frame.header_block_fragment_length();
//          for (auto it = continuation_frames.begin(); it != continuation_frames.end(); ++it)
//            serialized_header_block_sz += it->header_block_fragment_length();
//
//          serialized_header_block.reserve(serialized_header_block_sz);
//
//          serialized_header_block.append(incoming_headers_frame.header_block_fragment(), incoming_headers_frame.header_block_fragment_length());
//
//          for (auto it = continuation_frames.begin(); it != continuation_frames.end(); ++it)
//          {
//            serialized_header_block.append(it->header_block_fragment(), it->header_block_fragment_length());
//          }
//
//          header_block headers;
//          if (!header_block::deserialize(dec, serialized_header_block, headers))
//            ret = v2_errc::compression_error;
//          else
//          {
            if (this->state_ == stream_state::reserved_remote)
            {
              this->state_ = stream_state::half_closed_local;
            }
            else if (this->state_ == stream_state::idle)
            {
              this->state_ = (end_stream ? stream_state::half_closed_remote : stream_state::open );
            }
            else
            {
              if (end_stream)
                this->state_ = (this->state_ == stream_state::half_closed_local ? stream_state::closed : stream_state::half_closed_remote);
            }
            in_headers_.emplace(std::move(headers));

            std::experimental::coroutine_handle<> coro{nullptr};
            std::swap(coro, recv_headers_coro_);
            coro ? coro() : void();

              // TODO !!!
  //          RecvMsg generic_head(std::move(headers));
  //          if (incoming_header_is_informational(generic_head))
  //            this->on_informational_headers_ ? this->on_informational_headers_(std::move(generic_head)) : void();
  //          else if (!this->on_headers_called_)
  //          {
  //            this->on_headers_called_ = true;
  //            this->on_headers_ ? this->on_headers_(std::move(generic_head)) : void();
  //          }
  //          else
  //            this->on_trailers_ ? this->on_trailers_(std::move(generic_head)) : void();
  //
  //          if (this->state_ == stream_state::closed)
  //          {
  //            this->on_end_ ? this->on_end_() : void();
  //            this->on_close_ ? this->on_close_(v2_errc::no_error) : void();
  //          }
  //          else if (this->state_ == stream_state::half_closed_remote)
  //          {
  //            this->on_end_ ? this->on_end_() : void();
  //          }

 //         }
          break;
        }
        default:
        {
          // TODO: deal with frame / state_ mismatch
        }
      }
      return ret;
    }

    v2_errc connection::stream::handle_incoming_priority(std::uint8_t weight, std::uint32_t dependency_id)
    {
      // TODO: implement.
      return v2_errc::no_error;
    }

    v2_errc connection::stream::handle_incoming_rst_stream(std::uint32_t error_code)
    {
      v2_errc ret = v2_errc::no_error;

      if (this->state_ != stream_state::closed)
      {
        std::queue<data_frame> rmv;
        this->out_data_ = nullptr;
        this->state_= stream_state::closed;

        // TODO: resume coro
        //this->on_close_ ? this->on_close_(int_to_v2_errc(incoming_rst_stream_frame.error_code())) : void();
      }

      return ret;
    }

    v2_errc connection::stream::handle_incoming_push_promise(header_block&& headers, stream& idle_promised_stream)
    {
      v2_errc ret = v2_errc::no_error;

      switch (this->state_)
      {
        case stream_state::half_closed_local:
        case stream_state::open:
        {

          idle_promised_stream.state_ = stream_state::reserved_remote;
          // TODO !!!
          //SendMsg generic_head(std::move(headers));
          //this->on_push_promise_ ? this->on_push_promise_(std::move(generic_head), incoming_push_promise_frame.promised_stream_id()) : void();
          break;
        }
        case stream_state::closed:
        {
          if (true) // if stream was reset by me
          {
            if (true) //incoming_push_promise_frame.promised_stream_id() <= this->parent_connection_.last_newly_accepted_stream_id_)
            {
              //              this->parent_connection_.last_newly_accepted_stream_id_ = incoming_push_promise_frame.promised_stream_id();
              //              assert(this->parent_connection_.create_stream(this->parent_connection_.last_newly_accepted_stream_id_));
              //              auto it = this->parent_connection_.streams_.find(this->parent_connection_.last_newly_accepted_stream_id_);
              //              assert(it != this->parent_connection_.streams_.end());
              idle_promised_stream.state_ = stream_state::reserved_remote;
              // TODO: !!! idle_promised_stream.send_rst_stream_frame(v2_errc::refused_stream);
              //this->parent_connection_.send_reset_stream(this->parent_connection_.last_newly_accepted_stream_id_, errc::refused_stream);
            }
            else
            {
              // TODO: error promised stream_id is too low
            }
          }
          else
          {
            // TODO: handle error.
          }
          break;
        }
        default:
        {
          // TODO: deal with frame / state_ mismatch
        }
      }

      return ret;
    }

    v2_errc connection::stream::handle_incoming_window_update(std::int32_t window_size_increment)
    {
      if (window_size_increment == 0 || (window_size_increment + this->outgoing_window_size_) < this->outgoing_window_size_)
        this->send_reset(v2_errc::flow_control_error);
      else
        this->outgoing_window_size_ = (window_size_increment + this->outgoing_window_size_);
      return v2_errc::no_error;
    }
    //================================================================//

    //================================================================//
    const std::uint32_t connection::default_header_table_size      = 4096;
    const std::uint32_t connection::default_enable_push            = 1;
    const std::uint32_t connection::default_initial_window_size    = 65535;
    const std::uint32_t connection::default_max_frame_size         = 16384;

    const std::array<char,24> connection::http2_preface{{0x50, 0x52, 0x49, 0x20, 0x2a, 0x20, 0x48, 0x54, 0x54, 0x50, 0x2f, 0x32, 0x2e, 0x30, 0x0d, 0x0a, 0x0d, 0x0a, 0x53, 0x4d, 0x0d, 0x0a, 0x0d, 0x0a}};

    connection::connection(manifold::tls_socket&& sock, http::version http_version, std::function<future<void>(std::shared_ptr<connection::stream>)> on_new_stream_handler) :
      socket_(new tls_socket(std::move(sock))),
      hpack_encoder_(default_header_table_size),
      hpack_decoder_(default_header_table_size),
      outgoing_window_size_(default_initial_window_size),
      incoming_window_size_(default_initial_window_size),
      on_new_stream_(on_new_stream_handler),
      protocol_version_(http_version),
      send_loop_running_(false),
      closed_(false),
      is_server_(on_new_stream_handler != nullptr)
    {
      // header_table_size      = 0x1, // 4096
      // enable_push            = 0x2, // 1
      // max_concurrent_streams = 0x3, // (infinite)
      // initial_window_size    = 0x4, // 65535
      // max_frame_size         = 0x5, // 16384
      // max_header_list_size   = 0x6  // (infinite)
      this->local_settings_ =
        {
          { setting_code::header_table_size,   default_header_table_size },
          { setting_code::enable_push,         default_enable_push },
          { setting_code::initial_window_size, default_initial_window_size },
          { setting_code::max_frame_size,      default_max_frame_size }
        };

      this->peer_settings_ = this->local_settings_;

      run_v2_recv_loop();
      //asio::spawn(socket_->io_service(), std::bind(&connection::run_v2_send_loop, this, std::placeholders::_1));
    }

    connection::connection(manifold::non_tls_socket&& sock, http::version http_version, std::function<future<void>(std::shared_ptr<connection::stream>)> on_new_stream_handler) :
      socket_(new non_tls_socket(std::move(sock))),
      hpack_encoder_(default_header_table_size),
      hpack_decoder_(default_header_table_size),
      outgoing_window_size_(default_initial_window_size),
      incoming_window_size_(default_initial_window_size),
      on_new_stream_(on_new_stream_handler),
      protocol_version_(http_version),
      send_loop_running_(false),
      closed_(false),
      is_server_(on_new_stream_handler != nullptr)
    {
      // header_table_size      = 0x1, // 4096
      // enable_push            = 0x2, // 1
      // max_concurrent_streams = 0x3, // (infinite)
      // initial_window_size    = 0x4, // 65535
      // max_frame_size         = 0x5, // 16384
      // max_header_list_size   = 0x6  // (infinite)
      this->local_settings_ =
        {
          { setting_code::header_table_size,   default_header_table_size },
          { setting_code::enable_push,         default_enable_push },
          { setting_code::initial_window_size, default_initial_window_size },
          { setting_code::max_frame_size,      default_max_frame_size }
        };

      this->peer_settings_ = this->local_settings_;

      run_v2_recv_loop();
//      asio::spawn(socket_->io_service(), std::bind(&connection::run_v2_recv_loop, this, std::placeholders::_1));
//      asio::spawn(socket_->io_service(), std::bind(&connection::run_v2_send_loop, this, std::placeholders::_1));
    }

    http::version connection::version()
    {
      return protocol_version_;
    }

//    connection::stream::recv_headers_awaiter connection::recv_headers(std::uint32_t stream_id)
//    {
//      auto it = streams_.find(stream_id);
//      assert(it != streams_.end());
//      if (it == streams_.end())
//        return stream::recv_headers_awaiter{nullptr};
//      return it->second.recv_headers();
//    }
//
//    connection::stream::recv_data_awaiter connection::recv_data(std::uint32_t stream_id, char* dest, std::size_t sz)
//    {
//      auto it = streams_.find(stream_id);
//      assert(it != streams_.end());
//      if (it == streams_.end())
//        return stream::recv_data_awaiter{nullptr, dest, sz};
//      return it->second.recv_data(dest, sz);
//    }
//
//    connection::stream::send_headers_awaiter connection::send_headers(std::uint32_t stream_id, const header_block& headers, bool end_stream)
//    {
//      auto it = streams_.find(stream_id);
//      assert(it != streams_.end());
//      if (it == streams_.end())
//        return stream::send_headers_awaiter{nullptr};
//      return it->second.send_headers(headers, end_stream);
//    }
//
//    connection::stream::send_data_awaiter connection::send_data(std::uint32_t stream_id, const char* data, std::size_t sz, bool end_stream)
//    {
//      auto it = streams_.find(stream_id);
//      assert(it != streams_.end());
//      if (it == streams_.end())
//        return stream::send_data_awaiter{nullptr, 0};
//      return it->second.send_data(data, sz, end_stream);
//    }
//
//    void connection::send_reset(std::uint32_t stream_id, v2_errc ec)
//    {
//      auto it = streams_.find(stream_id);
//      assert(it != streams_.end());
//      if (it != streams_.end())
//        return it->second.send_reset(ec);
//    }


    future<void> connection::run_v2_recv_loop()
    {
      while (!this->closed_)
      {
        std::error_code ec;
        frame_payload frame(0, frame_type::invalid_type, 0x0, 0);
        co_await frame_payload::recv(*this->socket_, frame, ec);
        if (ec)
        {
          this->close(v2_errc::internal_error);
        }
        else
        {
          if (frame.stream_id())
          {
            auto current_stream_it = this->streams_.find(frame.stream_id());

            auto t = frame.type();
            auto id = frame.stream_id();

            if (current_stream_it == this->streams_.end() && frame.type() == frame_type::headers && is_server_ && (frame.stream_id() % 2) != 0)
            {
              if (frame.stream_id() > this->last_newly_accepted_stream_id_)
              {
                this->last_newly_accepted_stream_id_ = frame.stream_id();
                if (!this->streams_.emplace(frame.stream_id(), std::make_shared<stream>(this, frame.stream_id(), this->local_settings_[setting_code::initial_window_size], this->peer_settings_[setting_code::initial_window_size])).second)
                {
                  this->close(v2_errc::internal_error);
                }
                else
                {
                  current_stream_it = this->streams_.find(frame.stream_id());
                  std::shared_ptr<stream> foo(current_stream_it->second);
                  if (this->on_new_stream_)
                    this->on_new_stream_(current_stream_it->second);
                }
                assert(current_stream_it != this->streams_.end());
              }
              else
              {
                this->close(v2_errc::protocol_error);
              }
            }

            if (current_stream_it == this->streams_.end())
            {
              // TODO: Handle Error.
              //self->close(errc::protocol_error); //Ignoring for now. Sending frame resets leaves the possibility for streams to be closed while frames are in flight.
            }
            else
            {
              v2_errc handle_frame_conn_error = v2_errc::no_error;
              switch (frame.type())
              {
              case frame_type::headers:
              {
                headers_frame tmp(std::move(frame));
                std::string header_string(tmp.header_block_fragment(),  tmp.header_block_fragment_length());
                std::list<continuation_frame> continuations_frames;
                if (!(frame.flags() & frame_flag::end_headers))
                {
                  continuation_frame cframe;
                  do
                  {
                    co_await frame_payload::recv(*this->socket_, cframe, ec);
                    if (ec)
                    {
                      this->close(v2_errc::internal_error);
                    }
                    else if (cframe.stream_id() != frame.stream_id() || cframe.type() != frame_type::continuation)
                    {
                      this->close(v2_errc::protocol_error);
                    }
                    else
                    {
                      header_string + std::string(cframe.header_block_fragment(), cframe.header_block_fragment_length());
                    }
                  } while (!closed_ && !cframe.has_end_headers_flag());
                }

                if (!ec)
                {
                  std::list<hpack::header_field> headers;
                  hpack_decoder_.decode(header_string.begin(), header_string.end(), headers);
                  handle_frame_conn_error = current_stream_it->second->handle_incoming_headers(header_block(std::move(headers)), tmp.has_end_stream_flag());
                  //current_stream_it->second->handle_incoming_frame(frame, this->local_settings_[setting_code::initial_window_size], handle_frame_conn_error);
                }
                break;
              }
              case frame_type::data:
              {
                data_frame tmp(std::move(frame));
                std::size_t data_length = tmp.data_length();
                if (data_length > this->incoming_window_size_)
                  this->close(v2_errc::flow_control_error);
                else
                {
                  this->incoming_window_size_ -= data_length;
                  if (this->incoming_window_size_ < this->local_settings_[setting_code::initial_window_size] / 2)
                    this->send_connection_level_window_update(this->local_settings_[setting_code::initial_window_size] - this->incoming_window_size_);

                  handle_frame_conn_error = current_stream_it->second->handle_incoming_data(tmp.data(), tmp.data_length(), this->local_settings_[setting_code::initial_window_size], tmp.has_end_stream_flag());
                }
                break;
              }
              case frame_type::priority:
              {
                priority_frame tmp(std::move(frame));
                handle_frame_conn_error = current_stream_it->second->handle_incoming_priority(tmp.weight(), tmp.stream_dependency_id());
                break;
              }
              case frame_type::rst_stream:
              {
                rst_stream_frame tmp(std::move(frame));
                handle_frame_conn_error = current_stream_it->second->handle_incoming_rst_stream(tmp.error_code());
                break;
              }
              case frame_type::window_update:
              {
                window_update_frame tmp(std::move(frame));
                handle_frame_conn_error = current_stream_it->second->handle_incoming_window_update(tmp.window_size_increment());
                break;
              }
              default:
              {
                // ignore unknown frame type
              }
              }

              if (handle_frame_conn_error != v2_errc::no_error)
                this->close(handle_frame_conn_error);
            }
          }
          else
          {
            switch (frame.type())
            {
            case frame_type::settings:
              this->handle_incoming(settings_frame(std::move(frame)));
              break;
            case frame_type::ping:
              this->handle_incoming(ping_frame(std::move(frame)));
              break;
            case frame_type::goaway:
              this->handle_incoming(goaway_frame(std::move(frame)));
              break;
            case frame_type::window_update:
              this->handle_incoming(window_update_frame(std::move(frame)));
              break;
            default:
            {
              // TODO: error stream-only frame missing stream id_
            }
            }
          }
        }
      }
    }

    future<void> connection::run_v2_send_loop()
    {
      if (!this->send_loop_running_)
      {
        this->send_loop_running_ = true;
        std::error_code ec;

        if (this->closed_)
        {
          while (this->outgoing_frames_.size())
          {
            if (this->outgoing_frames_.front().type() == frame_type::goaway)
            {
              co_await frame_payload::send(*this->socket_, this->outgoing_frames_.front(), ec);
              outgoing_frames_.pop();

              this->socket_->close();

              streams_.clear();
            }
            this->outgoing_frames_.pop();
          }

          this->send_loop_running_ = false;
        }
        else
        {
          while (send_loop_running_ && !closed_)
          {
            if (this->outgoing_frames_.size())
            {
              // TODO: this->data_transfer_deadline_timer_.expires_from_now(this->data_transfer_timeout_);
              co_await frame_payload::send(*this->socket_, this->outgoing_frames_.front(), ec);
              this->outgoing_frames_.pop();
            }
            else
            {
              auto stream_it = this->find_sendable_stream(outgoing_window_size_ == 0);
              if (stream_it != this->streams_.end())
              {
                auto& s = *stream_it;
                if (stream_it->second->has_sendable_headers())
                {
                  std::string header_string;
                  hpack_encoder_.encode(stream_it->second->sendable_headers().raw_headers(), header_string);
                  bool end_stream = ((stream_it->second->state() == stream_state::half_closed_local || stream_it->second->state() == stream_state::closed) && stream_it->second->out_data_size() == 0);
                  headers_frame frame(stream_it->first, header_string.data(), header_string.size(), true, end_stream);
                  co_await frame_payload::send(*socket_, frame, ec);
                  if (ec)
                  {
                    this->close(v2_errc::internal_error);
                    std::cout << "ERROR " << __FILE__ << ":" << __LINE__ << " " << ec.message() << std::endl;
                  }
                  stream_it->second->pop_sendable_headers();
                }

                if (!ec && outgoing_window_size_ && stream_it->second->has_sendable_data())
                {
                  const char* data;
                  std::size_t data_sz;
                  std::tie(data, data_sz) = stream_it->second->sendable_data();

                  if (data_sz > outgoing_window_size_)
                    data_sz = outgoing_window_size_;

                  outgoing_window_size_ -= data_sz;

                  bool end_stream = ((stream_it->second->state() == stream_state::half_closed_local || stream_it->second->state() == stream_state::closed) && stream_it->second->out_data_size() == data_sz);

                  data_frame frame(stream_it->first, data, data_sz, end_stream);
                  co_await frame_payload::send(*socket_, frame, ec);
                  if (ec)
                  {
                    this->close(v2_errc::internal_error);
                    std::cout << "ERROR " << __FILE__ << ":" << __LINE__ << " " << ec.message() << std::endl;
                  }
                  stream_it->second->pop_sendable_data(data_sz);
                }

                if (!ec && stream_it->second->has_sendable_window_update())
                {
                  std::uint32_t amount = stream_it->second->sendable_window_update();
                  window_update_frame frame(stream_it->first, amount);
                  co_await frame_payload::send(*socket_, frame, ec);
                  if (ec)
                  {
                    this->close(v2_errc::internal_error);
                    std::cout << "ERROR " << __FILE__ << ":" << __LINE__ << " " << ec.message() << std::endl;
                  }
                  stream_it->second->pop_sendable_window_update();
                }
              }
              else
              {
                send_loop_running_ = false;
              }
            }
          }
        }
      }
    }

    v2_errc connection::handle_incoming(settings_frame&& incoming_frame)
    {
      v2_errc ret = v2_errc::no_error;
      if (incoming_frame.has_ack_flag())
      {
        if (this->pending_local_settings_.empty())
        {
          this->close(v2_errc::protocol_error);
        }
        else
        {
          auto& settings_list = this->pending_local_settings_.front();
          for (auto it = settings_list.begin(); it != settings_list.end(); ++it)
          {
            switch (it->first)
            {
            case (std::uint16_t)setting_code::header_table_size:
              this->local_settings_[setting_code::header_table_size] = it->second;
              break;
            case (std::uint16_t)setting_code::enable_push:
              this->local_settings_[setting_code::enable_push] = it->second;
              break;
            case (std::uint16_t)setting_code::max_concurrent_streams:
              this->local_settings_[setting_code::max_concurrent_streams] = it->second;
              break;
            case (std::uint16_t)setting_code::initial_window_size:
              if (it->second > 0x7FFFFFFF)
              {
                this->close(v2_errc::flow_control_error);
              }
              else
              {
                for (auto s = this->streams_.begin(); s != this->streams_.end(); ++s)
                {
                  if (!s->second->adjust_local_window_size(static_cast<std::int32_t>(it->second) - static_cast<std::int32_t>(this->local_settings_[setting_code::initial_window_size])))
                  {
                    this->close(v2_errc::flow_control_error);
                    break;
                  }
                }
              }
              this->local_settings_[setting_code::initial_window_size] = it->second;
              break;
            case (std::uint16_t)setting_code::max_frame_size:
              this->local_settings_[setting_code::max_frame_size] = it->second;
              break;
            case (std::uint16_t)setting_code::max_header_list_size:
              this->local_settings_[setting_code::max_header_list_size] = it->second;
              break;
            }
          }
        }
      }
      else
      {
        std::list<std::pair<std::uint16_t, std::uint32_t>> settings_list(incoming_frame.settings());
        std::cout << settings_list.size() << std::endl;
        for (auto it = settings_list.begin(); it != settings_list.end(); ++it)
        {
          switch (it->first)
          {
          case (std::uint16_t)setting_code::header_table_size:
            this->peer_settings_[setting_code::header_table_size] = it->second;
            if (it->second < 4096)
              this->hpack_encoder_.add_table_size_update(it->second);
            else
              this->hpack_encoder_.add_table_size_update(4096);
            break;
          case (std::uint16_t)setting_code::enable_push:
            this->peer_settings_[setting_code::enable_push] = it->second;
            break;
          case (std::uint16_t)setting_code::max_concurrent_streams:
            this->peer_settings_[setting_code::max_concurrent_streams] = it->second;
            break;
          case (std::uint16_t)setting_code::initial_window_size:
            if (it->second > 0x7FFFFFFF)
            {
              this->close(v2_errc::flow_control_error);
            }
            else
            {
              for (auto s = this->streams_.begin(); s != this->streams_.end(); ++s)
              {
                if (!s->second->adjust_peer_window_size(static_cast<std::int32_t>(it->second) - static_cast<std::int32_t>(this->peer_settings_[setting_code::initial_window_size])))
                {
                  this->close(v2_errc::flow_control_error);
                  break;
                }
              }
            }
            this->peer_settings_[setting_code::initial_window_size] = it->second;
            break;
          case (std::uint16_t)setting_code::max_frame_size:
            this->peer_settings_[setting_code::max_frame_size] = it->second;
            break;
          case (std::uint16_t)setting_code::max_header_list_size:
            this->peer_settings_[setting_code::max_header_list_size] = it->second;
            break;
          }
        }

        this->outgoing_frames_.emplace(settings_frame(ack_flag()));
      }

      return ret;
    }

    void connection::send_connection_level_window_update(std::int32_t amount)
    {
      outgoing_frames_.emplace(window_update_frame(0, amount));
    }

    void connection::close(v2_errc ec)
    {
      std::uint32_t last_stream_id = 0; // TODO:
      outgoing_frames_.emplace(goaway_frame(last_stream_id, ec, nullptr, 0));
    }

    std::unordered_map<std::uint32_t, std::shared_ptr<connection::stream>>::iterator connection::find_sendable_stream(bool exclude_data)
    {
      if (streams_.empty())
        return streams_.end();

      auto random_stream_it = streams_.begin();
      std::advance(random_stream_it, this->rng_() % this->streams_.size());

      for (auto it = random_stream_it; it != this->streams_.end(); ++it)
      {
        if (it->second->has_something_to_send(exclude_data))
          return it;
      }


      for (auto it = this->streams_.begin(); it != random_stream_it; ++it)
      {
        if (it->second->has_something_to_send(exclude_data))
          return it;
      }

      return this->streams_.end();
    }

    v2_errc connection::handle_incoming(ping_frame&& incoming_frame)
    {
      // TODO: SEND PING !!!
//      this->outgoing_ping_frames_.emplace(incoming_frame.data(), true);
//      http::ping_frame& f = this->outgoing_ping_frames_.back();
//      this->outgoing_frame_headers_.emplace(f.serialized_length(), frame_type::ping, f.flags(), 0);
      return v2_errc::no_error;
    }

    v2_errc connection::handle_incoming(goaway_frame&& incoming_frame)
    {
      auto c = incoming_frame.error_code();
      std::string s(incoming_frame.additional_debug_data(), incoming_frame.additional_debug_data_length());
      auto sz = s.size();
      return v2_errc::no_error;
    }

    v2_errc connection::handle_incoming(window_update_frame&& incoming_frame)
    {
      std::int32_t amount = incoming_frame.window_size_increment();
      if (amount <= 0 || (amount + this->outgoing_window_size_) < this->outgoing_window_size_)
      {
        this->close(v2_errc::flow_control_error);
        return v2_errc::flow_control_error;
      }

      this->outgoing_window_size_ = (amount + this->outgoing_window_size_);
      return v2_errc::no_error;
    }

    void connection::spawn_v2_send_loop_if_needed() // TODO: revisit corner cases.
    {
      if (!this->send_loop_running_)
        connection::run_v2_send_loop();
    }
    //================================================================//
  }
}