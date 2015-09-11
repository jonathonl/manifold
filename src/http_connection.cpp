
#include <iostream>

#include "http_connection.hpp"

namespace manifold
{
  namespace http
  {
    //----------------------------------------------------------------//
    connection::stream_dependency_tree::stream_dependency_tree(stream* stream_ptr)
      : stream_ptr_(stream_ptr)
    {
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    connection::stream_dependency_tree::stream_dependency_tree(stream* stream_ptr, const std::vector<stream_dependency_tree>& children)
      : children_(children), stream_ptr_(stream_ptr)
    {
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    connection::stream* connection::stream_dependency_tree::stream_ptr() const
    {
      return this->stream_ptr_;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    const std::vector<connection::stream_dependency_tree>& connection::stream_dependency_tree::children() const
    {
      return this->children_;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void connection::stream_dependency_tree::insert_child(connection::stream_dependency_tree&& child)
    {
      this->children_.push_back(std::move(child));
    }
    //----------------------------------------------------------------//

    void connection::stream_dependency_tree::remove(stream& stream_to_remove)
    {
      bool found = false;
      for (auto it = this->children_.begin(); it != this->children_.end() && found == false; ++it)
      {
        if (it->stream_ptr() == &stream_to_remove)
        {
          stream_dependency_tree tree_to_remove(std::move(*it));
          this->children_.erase(it);

          this->children_.reserve(this->children_.size() + tree_to_remove.children_.size());
          for (auto child_it = tree_to_remove.children_.begin(); child_it != tree_to_remove.children_.end(); ++child_it)
          {
            this->children_.push_back(std::move(*child_it));
          }
          tree_to_remove.children_.clear();
          found = true;
        }
      }

      for (auto it = this->children_.begin(); it != this->children_.end() && found == false; ++it)
        it->remove(stream_to_remove);
    }

    //----------------------------------------------------------------//
    connection::connection(connection_io_impl* io_impl)
      : io_impl_(io_impl), hpack_encoder_(4096), hpack_decoder_(4096), last_newly_accepted_stream_id_(0), last_newly_created_stream_id_(0), root_stream_(0), stream_dependency_tree_(&this->root_stream_)
    void connection::stream_dependency_tree::clear_children()
    {
      for (auto it = this->children_.begin(); it != this->children_.end(); ++it)
        it->clear_children();
      this->children_.clear();
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    const std::array<char,24> connection::preface{{0x50,0x52,0x49,0x20,0x2a,0x20,0x48,0x54,0x54,0x50,0x2f,0x32,0x2e,0x30,0x0d,0x0a,0x0d,0x0a,0x53,0x4d,0x0d,0x0a,0x0d,0x0a}};
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    connection::connection()
    : hpack_encoder_(4096), hpack_decoder_(4096), last_newly_accepted_stream_id_(0), last_newly_created_stream_id_(0), root_stream_(*this, 0), stream_dependency_tree_(&this->root_stream_)
    {
      std::seed_seq seed({static_cast<std::uint32_t>((std::uint64_t)this)});
      this->rg_.seed(seed);


      // header_table_size      = 0x1, // 4096
      // enable_push            = 0x2, // 1
      // max_concurrent_streams = 0x3, // (infinite)
      // initial_window_size    = 0x4, // 65535
      // max_frame_size         = 0x5, // 16384
      // max_header_list_size   = 0x6  // (infinite)
      this->settings_ =
        {
          { setting_code::header_table_size,    4096 },
          { setting_code::enable_push,             1 },
          { setting_code::initial_window_size, 65535 },
          { setting_code::max_frame_size,      16384 }
        };

      this->started_ = false;
      this->closed_ = false;
      this->send_loop_running_ = false;

    };
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    connection::~connection()
    {
      std::cout << "~connection()" << std::endl;
      //this->socket().close();
    }
    //----------------------------------------------------------------//

    void connection::close()
    {
      if (!this->closed_)
      {
        this->closed_ = true;
        if (this->on_close_)
          this->on_close_(0); //TODO: Set error code when applicable.
        this->on_close_ = nullptr;
        this->on_new_stream_ = nullptr;
        this->stream_dependency_tree_.clear_children();
        this->streams_.clear();
        this->socket().close();
      }
    }

    //----------------------------------------------------------------//
    bool connection::stream_has_sendable_frame(const stream& stream_to_check)
    {
      return (stream_to_check.outgoing_non_data_frames.size()
        || (stream_to_check.outgoing_data_frames.size() && stream_to_check.outgoing_window_size > 0 && this->connection_level_outgoing_window_size() > 0)
        || (stream_to_check.outgoing_data_frames.size() && stream_to_check.outgoing_data_frames.front().payload_length() == 0));
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    bool connection::check_tree_for_outgoing_frame(const stream_dependency_tree& current_node)
    {
      bool ret = false;

      if (this->stream_has_sendable_frame(*current_node.stream_ptr()))
        ret = true;

      for (auto it = current_node.children().begin(); !ret && it != current_node.children().end(); ++it)
      {
        ret = check_tree_for_outgoing_frame(*it);
      }

      return ret;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    connection::stream* connection::get_next_send_stream_ptr(const stream_dependency_tree& current_node)
    {
      // TODO: enforce a max tree depth of 10 to avoid stack overflow from recursion.
      stream* ret = nullptr;

      if (this->stream_has_sendable_frame(*current_node.stream_ptr()))
      {
        ret = current_node.stream_ptr();
      }
      else
      {
        std::uint64_t weight_sum = 0;
        std::vector<const stream_dependency_tree*> pool;

        for (auto it = current_node.children().begin(); it != current_node.children().end(); ++it)
        {
          if (this->check_tree_for_outgoing_frame(*it))
          {
            pool.push_back(&(*it));
            weight_sum += (it->stream_ptr()->weight + 1);
          }
        }

        if (pool.size())
        {
          std::uint64_t sum_index = (this->rg_() % weight_sum) + 1;
          std::uint64_t current_sum = 0;
          for (auto it = pool.begin(); it != pool.end(); ++it)
          {
            const stream_dependency_tree* current_pool_node = (*it);
            current_sum += (current_pool_node->stream_ptr()->weight + 1);
            if (sum_index <= current_sum)
            {
              ret = this->get_next_send_stream_ptr(*current_pool_node);
            }
          }
        }
      }

      return ret;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void connection::garbage_collect_streams()
    {
      for (auto it = this->streams_.begin(); it != this->streams_.end(); )
      {
        if (it->second.state() == stream_state::closed)
        {
          this->stream_dependency_tree_.remove(it->second);
          it = this->streams_.erase(it);
        }
        else
        {
          ++it;
        }
      }
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void connection::run_recv_loop()
    {
      std::shared_ptr<connection> self = this->shared_from_this();
      this->incoming_frame_ = frame(); // reset incoming frame
      this->io_impl_->recv_frame(this->incoming_frame_, [self](const std::error_code& ec)
      {
        if (ec)
        {
          // TODO: Handle error.
          std::cout << ec.message() << std::endl;
        }
        else
        {
          const frame_type incoming_frame_type = self->incoming_frame_.type();
          std::uint32_t incoming_stream_id = self->incoming_frame_.stream_id();
          if (incoming_stream_id)
          {
            std::map<std::uint32_t,stream>::iterator current_stream_it = self->streams_.find(incoming_stream_id);

            if (current_stream_it == self->streams_.end())
            {
              if (incoming_stream_id > self->last_newly_accepted_stream_id_)
              {
                self->last_newly_accepted_stream_id_ = incoming_stream_id;
                if (self->create_stream(incoming_stream_id))
                  current_stream_it = self->streams_.find(incoming_stream_id);
                assert(current_stream_it != self->streams_.end());
              }
              else
              {
                // TODO: Connection error for reusing old stream id_.
              }
            }

            if (current_stream_it == self->streams_.end())
            {
              // TODO: Handle Error.
            }
            else
            {
              if (self->incoming_header_block_fragments_.size() && (incoming_frame_type != frame_type::continuation || self->incoming_frame_.stream_id() != self->incoming_header_block_fragments_.front().stream_id()))
              {
                // TODO: connection error PROTOCOL_ERROR
              }
              else if (incoming_frame_type == frame_type::continuation)
              {
                if (self->incoming_header_block_fragments_.empty())
                {
                  // TODO: connection error PROTOCOL_ERROR
                }
                else if (!self->incoming_frame_.continuation_frame().has_end_headers_flag())
                {
                  self->incoming_header_block_fragments_.push(std::move(self->incoming_frame_));
                }
                else
                {
                  if (self->incoming_header_block_fragments_.front().type() == frame_type::headers)
                  {
                    headers_frame h_frame(std::move(self->incoming_header_block_fragments_.front().headers_frame()));
                    self->incoming_header_block_fragments_.pop();

                    std::vector<continuation_frame> cont_frames;
                    cont_frames.reserve(self->incoming_header_block_fragments_.size());
                    while (self->incoming_header_block_fragments_.size())
                    {
                      cont_frames.push_back(std::move(self->incoming_header_block_fragments_.front().continuation_frame()));
                      self->incoming_header_block_fragments_.pop();
                    }

                    current_stream_it->second.handle_incoming_frame(h_frame, cont_frames);
                  }
                  else
                  {
                    push_promise_frame pp_frame(std::move(self->incoming_header_block_fragments_.front().push_promise_frame()));
                    self->incoming_header_block_fragments_.pop();

                    std::vector<continuation_frame> cont_frames;
                    cont_frames.reserve(self->incoming_header_block_fragments_.size());
                    while (self->incoming_header_block_fragments_.size())
                    {
                      cont_frames.push_back(std::move(self->incoming_header_block_fragments_.front().continuation_frame()));
                      self->incoming_header_block_fragments_.pop();
                    }

                    current_stream_it->second.handle_incoming_frame(pp_frame, cont_frames);
                  }
                }
              }
              else if (incoming_frame_type == frame_type::headers || incoming_frame_type == frame_type::push_promise)
              {
                bool has_end_headers_flag = (incoming_frame_type == frame_type::headers ? self->incoming_frame_.headers_frame().has_end_headers_flag() : self->incoming_frame_.push_promise_frame().has_end_headers_flag());

                if (!has_end_headers_flag)
                  self->incoming_header_block_fragments_.push(std::move(self->incoming_frame_));
                else if (incoming_frame_type == frame_type::headers)
                  current_stream_it->second.handle_incoming_frame(self->incoming_frame_.headers_frame(), {});
                else
                  current_stream_it->second.handle_incoming_frame(self->incoming_frame_.push_promise_frame(), {});
              }
              else
              {
                switch (incoming_frame_type)
                {
                  case frame_type::data:
                    current_stream_it->second.handle_incoming_frame(self->incoming_frame_.data_frame());
                    break;
                  case frame_type::priority:
                    current_stream_it->second.handle_incoming_frame(self->incoming_frame_.priority_frame());
                    break;
                  case frame_type::rst_stream:
                    current_stream_it->second.handle_incoming_frame(self->incoming_frame_.rst_stream_frame());
                    break;
                  case frame_type::window_update:
                    current_stream_it->second.handle_incoming_frame(self->incoming_frame_.window_update_frame());
                    break;
                  default:
                  {
                    // TODO: Handle error. connection-only frame type has stream id_.
                  }
                }
              }
            }
          }
          else
          {
            switch (incoming_frame_type)
            {
              case frame_type::settings:
                self->handle_incoming_frame(self->incoming_frame_.settings_frame());
                break;
              case frame_type::ping:
                self->handle_incoming_frame(self->incoming_frame_.ping_frame());
                break;
              case frame_type::goaway:
                self->handle_incoming_frame(self->incoming_frame_.goaway_frame());
                break;
              case frame_type::window_update:
                self->handle_incoming_frame(self->incoming_frame_.window_update_frame());
                break;
              default:
              {
                // TODO: error stream-only frame missing stream id_
              }
            }
          }

          self->run_recv_loop();
        }
      });
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void connection::stream::handle_incoming_frame(const data_frame& incoming_data_frame)
    {
      switch (this->state_)
      {
        case stream_state::open:
        {
          this->on_data_ ? this->on_data_(incoming_data_frame.data(), incoming_data_frame.data_length()) : void();

          if (incoming_data_frame.has_end_stream_flag())
          {
            this->state_ = stream_state::half_closed_remote;
            this->on_end_ ? this->on_end_() : void();
          }
          break;
        }
        case stream_state::half_close_local:
        {
          this->on_data_ ? this->on_data_(incoming_data_frame.data(), incoming_data_frame.data_length()) : void();

          if (incoming_data_frame.has_end_stream_flag())
          {
            this->state_= stream_state::closed;
            this->on_end_ ? this->on_end_() : void();
            this->on_close_ ? this->on_close_(0) : void();
          }
          break;
        }
        default:
        {
          // TODO: deal with frame / state_ mismatch
        }
      }
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void connection::stream::handle_incoming_frame(const headers_frame& incoming_headers_frame, const std::vector<continuation_frame>& continuation_frames)
    {
      switch (this->state_)
      {
        case stream_state::idle:
        case stream_state::open:
        case stream_state::half_close_local:
        {
          header_block headers;

          {
            std::string header_data(incoming_headers_frame.header_block_fragment(), incoming_headers_frame.header_block_fragment_length());
            header_block::deserialize(this->parent_connection_.hpack_decoder_, header_data, headers);
          }

          for (auto it = continuation_frames.begin(); it != continuation_frames.end(); ++it)
          {
            std::string header_data(it->header_block_fragment(), it->header_block_fragment_length());
            header_block::deserialize(this->parent_connection_.hpack_decoder_, header_data, headers);
          }


          if (this->state_ == stream_state::idle)
          {
            this->state_ = (incoming_headers_frame.has_end_stream_flag() ? stream_state::half_closed_remote : stream_state::open );
          }
          else
          {
            if (incoming_headers_frame.has_end_stream_flag())
              this->state_ = (this->state_ == stream_state::half_close_local ? stream_state::closed : stream_state::half_closed_remote);
          }
          this->on_headers_ ? this->on_headers_(std::move(headers)) : void();

          if (this->state_ == stream_state::closed)
          {
            this->on_end_ ? this->on_end_() : void();
            this->on_close_ ? this->on_close_(0) : void();
          }
          else if (this->state_ == stream_state::half_closed_remote)
          {
            this->on_end_ ? this->on_end_() : void();
          }


          break;
        }
        default:
        {
          // TODO: deal with frame / state_ mismatch
        }
      }
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void connection::stream::handle_incoming_frame(const priority_frame& incoming_priority_frame)
    {
      // TODO: implement.
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void connection::stream::handle_incoming_frame(const rst_stream_frame& incoming_rst_stream_frame)
    {
      if (this->state_ != stream_state::closed)
      {
        this->state_= stream_state::closed;
        this->on_close_ ? this->on_close_(incoming_rst_stream_frame.error_code()) : void();
      }
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void connection::handle_incoming_frame(const settings_frame& incoming_settings_frame)
    {
      // TODO
    }
    //----------------------------------------------------------------//

    void connection::stream::handle_incoming_frame(const push_promise_frame& incoming_push_promise_frame, const std::vector<continuation_frame>& continuation_frames)
    {
      switch (this->state_)
      {
        case stream_state::half_close_local:
        case stream_state::open:
        {
          header_block headers;

          {
            std::string header_data(incoming_push_promise_frame.header_block_fragment(), incoming_push_promise_frame.header_block_fragment_length());
            header_block::deserialize(this->parent_connection_.hpack_decoder_, header_data, headers);
          }

          for (auto it = continuation_frames.begin(); it != continuation_frames.end(); ++it)
          {
            std::string header_data(it->header_block_fragment(), it->header_block_fragment_length());
            header_block::deserialize(this->parent_connection_.hpack_decoder_, header_data, headers);
          }

          if (incoming_push_promise_frame.promised_stream_id() <= this->parent_connection_.last_newly_accepted_stream_id_)
          {
            this->parent_connection_.last_newly_accepted_stream_id_ = incoming_push_promise_frame.promised_stream_id();
            assert(this->parent_connection_.create_stream(this->parent_connection_.last_newly_accepted_stream_id_));
            auto it = this->parent_connection_.streams_.find(this->parent_connection_.last_newly_accepted_stream_id_);
            assert(it != this->parent_connection_.streams_.end());
            it->second.state_ = stream_state::reserved_remote;
            this->on_push_promise_ ? this->on_push_promise_(std::move(headers), this->parent_connection_.last_newly_accepted_stream_id_) : void();
          }
          else
          {
            // TODO: error promised stream_id is too low
          }
        }
        case stream_state::closed:
        {
          if (true) // if stream was reset by me
          {
            if (incoming_push_promise_frame.promised_stream_id() <= this->parent_connection_.last_newly_accepted_stream_id_)
            {
              this->parent_connection_.last_newly_accepted_stream_id_ = incoming_push_promise_frame.promised_stream_id();
              assert(this->parent_connection_.create_stream(this->parent_connection_.last_newly_accepted_stream_id_));
              auto it = this->parent_connection_.streams_.find(this->parent_connection_.last_newly_accepted_stream_id_);
              assert(it != this->parent_connection_.streams_.end());
              it->second.state_ = stream_state::reserved_remote;
              this->parent_connection_.send_reset_stream(this->parent_connection_.last_newly_accepted_stream_id_, errc::refused_stream);
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
        }
        default:
        {
          // TODO: deal with frame / state_ mismatch
        }
      }
    }

    void connection::handle_incoming_frame(const ping_frame& incoming_ping_frame)
    {
      this->send_ping_acknowledgement(incoming_ping_frame.data());
    }

    void connection::handle_incoming_frame(const goaway_frame& incoming_goaway_frame)
    {
      // TODO:
    }

    //----------------------------------------------------------------//
    void connection::handle_incoming_frame(const window_update_frame& incoming_window_update_frame)
    {
      this->root_stream_.outgoing_window_size += incoming_window_update_frame.window_size_increment();
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void connection::stream::handle_incoming_frame(const window_update_frame& incoming_window_update_frame)
    {
      this->outgoing_window_size += incoming_window_update_frame.window_size_increment();
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    bool connection::stream::handle_outgoing_headers_state_change()
    {
      switch (this->state_)
      {
        case stream_state::idle:
          this->state_ = stream_state::open;
          return true;
        case stream_state::reserved_local:
          this->state_ = stream_state::half_closed_remote;
          return true;
        case stream_state::reserved_remote:
        case stream_state::half_close_local:
        case stream_state::closed:
          return false;
        case stream_state::open:
        case stream_state::half_closed_remote:
          return true;
      }
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    bool connection::stream::handle_outgoing_end_stream_state_change()
    {
      switch (this->state_)
      {
        case stream_state::open:
          this->state_ = stream_state::half_close_local;
          return true;
        case stream_state::half_closed_remote:
          this->state_ = stream_state::closed;
          return true;
        case stream_state::reserved_remote:
        case stream_state::idle:
        case stream_state::reserved_local:
        case stream_state::half_close_local:
        case stream_state::closed:
          return false;
      }
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    bool connection::stream::handle_outgoing_rst_stream_state_change()
    {
      switch (this->state_)
      {
        case stream_state::idle:
        case stream_state::closed:
          return false;
        default:
          this->state_ = stream_state::closed;
          return true;
      }
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void connection::run_send_loop()
    {
      if (!this->send_loop_running_)
      {
        this->send_loop_running_ = true;

        auto self = shared_from_this();

        stream* prioritized_stream_ptr = this->get_next_send_stream_ptr(this->stream_dependency_tree_);

        if (prioritized_stream_ptr)
        {
          if (prioritized_stream_ptr->outgoing_non_data_frames.size())
          {
            this->outgoing_frame_ = std::move(prioritized_stream_ptr->outgoing_non_data_frames.front());
            prioritized_stream_ptr->outgoing_non_data_frames.pop();
          }
          else
          {
            this->outgoing_frame_ = std::move(prioritized_stream_ptr->outgoing_data_frames.front());
            prioritized_stream_ptr->outgoing_data_frames.pop();
          }


          this->send_frame(this->outgoing_frame_, [self](const std::error_code& ec)
          {
            self->send_loop_running_ = false;
            if (ec)
            {
              // TODO: Handle error.
            }
            else
            {
              self->run_send_loop();
            }
          });
        }
        else
        {
          // All pending frames are sent so cleanup.
          this->garbage_collect_streams();
          this->send_loop_running_ = false;
        }
      }
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void connection::run()
    {
      if (!this->started_)
      {
        this->run_recv_loop();

        this->started_ = true;
      }
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void connection::on_new_stream(const std::function<void(std::int32_t stream_id)>& fn)
    {
      this->on_new_stream_ = fn;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void connection::on_close(const std::function<void(std::uint32_t ec)>& fn)
    {
      this->on_close_ = fn;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void connection::on_data(std::uint32_t stream_id, const std::function<void(const char* const buf, std::size_t buf_size)>& fn)
    {
      std::map<std::uint32_t,stream>::iterator it = this->streams_.find(stream_id);
      if (it == this->streams_.end())
      {
        // TODO: Handle error
      }
      else
      {
        it->second.on_data(fn);
      }
    }
    //----------------------------------------------------------------//

    void connection::on_headers(std::uint32_t stream_id, const std::function<void(http::header_block&& headers)>& fn)
    {
      std::map<std::uint32_t,stream>::iterator it = this->streams_.find(stream_id);
      if (it == this->streams_.end())
      {
        // TODO: Handle error
      }
      else
      {
        it->second.on_headers(fn);
      }
    }

    void connection::on_close(std::uint32_t stream_id, const std::function<void(std::uint32_t error_code)>& fn)
    {
      std::map<std::uint32_t,stream>::iterator it = this->streams_.find(stream_id);
      if (it == this->streams_.end())
      {
        // TODO: Handle error
      }
      else
      {
        it->second.on_close(fn);
      }
    }

    void connection::on_push_promise(std::uint32_t stream_id, const std::function<void(http::header_block&& headers, std::uint32_t promised_stream_id)>& fn)
    {
      std::map<std::uint32_t,stream>::iterator it = this->streams_.find(stream_id);
      if (it == this->streams_.end())
      {
        // TODO: Handle error
      }
      else
      {
        it->second.on_push_promise(fn);
      }
    }

    //----------------------------------------------------------------//
    void connection::on_end(std::uint32_t stream_id, const std::function<void()>& fn)
    {
      std::map<std::uint32_t,stream>::iterator it = this->streams_.find(stream_id);
      if (it == this->streams_.end())
      {
        // TODO: Handle error
      }
      else
      {
        it->second.on_end(fn);
      }
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void connection::on_drain(std::uint32_t stream_id, const std::function<void()>& fn)
    {
      std::map<std::uint32_t,stream>::iterator it = this->streams_.find(stream_id);
      if (it == this->streams_.end())
      {
        // TODO: Handle error
      }
      else
      {
        it->second.on_drain(fn);
      }
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    bool connection::create_stream(std::uint32_t stream_id) //TODO: allow for dependency other than root.
    {
      bool ret = false;

      std::pair<std::map<std::uint32_t,stream>::iterator,bool> insert_res = this->streams_.emplace(stream_id, stream(*this, stream_id));
      if (insert_res.second)
      {
        this->stream_dependency_tree_.insert_child(stream_dependency_tree((&insert_res.first->second)));

        ret = true;
      }

      return ret;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    bool connection::send_headers(std::uint32_t stream_id, const header_block&head, bool end_headers, bool end_stream)
    {
      bool ret = false;

      std::map<std::uint32_t,stream>::iterator it = this->streams_.find(stream_id);
      if (it == this->streams_.end())
      {
        // TODO: Handle error
      }
      else
      {
        std::string header_data;
        http::header_block::serialize(this->hpack_encoder_, head, header_data);
        const std::uint8_t EXTRA_BYTE_LENGTH_NEEDED_FOR_HEADERS_FRAME = 0; //TODO: Set correct value
        if ((header_data.size() + EXTRA_BYTE_LENGTH_NEEDED_FOR_HEADERS_FRAME) > this->settings_[setting_code::max_frame_size])
        {
          // TODO: Handle error
        }
        else
        {
          // TODO: check for errors below
          bool state_change_result = it->second.handle_outgoing_headers_state_change();
          if (end_stream && state_change_result)
            state_change_result = it->second.handle_outgoing_end_stream_state_change();

          if (state_change_result)
          {
            it->second.outgoing_non_data_frames.push(http::frame(http::headers_frame(header_data.data(), (std::uint32_t)header_data.size(), end_headers, end_stream), stream_id));
            this->run_send_loop();
            ret = true;
          }
          else
          {
            assert(!"Stream state_ change not allowed.");
          }
        }
      }

      return ret;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    bool connection::send_headers(std::uint32_t stream_id, const header_block& head, priority_options priority, bool end_headers, bool end_stream)
    {
      bool ret = false;

      std::map<std::uint32_t,stream>::iterator it = this->streams_.find(stream_id);

      if (it == this->streams_.end())
      {
        // TODO: Handle error
      }
      else
      {
        std::string header_data;
        http::header_block::serialize(this->hpack_encoder_, head, header_data);
        const std::uint8_t EXTRA_BYTE_LENGTH_NEEDED_FOR_HEADERS_FRAME = 0; //TODO: Set correct value
        if ((header_data.size() + EXTRA_BYTE_LENGTH_NEEDED_FOR_HEADERS_FRAME) > this->settings_[setting_code::max_frame_size])
        {
          // TODO: Handle error
        }
        else
        {
          // TODO: check for errors below
          auto state_change_result = it->second.handle_outgoing_headers_state_change();
          if (end_stream && state_change_result)
            state_change_result = it->second.handle_outgoing_end_stream_state_change();

          if (state_change_result)
          {
            it->second.outgoing_non_data_frames.push(http::frame(http::headers_frame(header_data.data(), (std::uint32_t)header_data.size(), end_headers, end_stream, priority), stream_id));
            this->run_send_loop();
            ret = true;
          }
          else
          {
            assert(!"Stream state_ change not allowed.");
          }
        }
      }

      return ret;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    bool connection::send_priority(std::uint32_t stream_id, priority_options options)
    {
      bool ret = false;

      std::map<std::uint32_t,stream>::iterator it = this->streams_.find(stream_id);
      if (it == this->streams_.end())
      {
        it->second.outgoing_non_data_frames.push(http::frame(http::priority_frame(options), stream_id));
      }

      return ret;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    bool connection::send_reset_stream(std::uint32_t stream_id, http::errc error_code)
    {
      bool ret = false;

      std::map<std::uint32_t,stream>::iterator it = this->streams_.find(stream_id);
      if (it == this->streams_.end())
      {
        if (it->second.handle_outgoing_rst_stream_state_change())
        {
          it->second.outgoing_non_data_frames.push(http::frame(http::rst_stream_frame(error_code), stream_id));
          this->run_send_loop();
          ret = true;
        }
        else
        {
          assert(!"Stream state_ change not allowed.");
        }
      }

      return ret;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void connection::send_settings(const std::list<std::pair<std::uint16_t,std::uint32_t>>& settings)
    {
      this->root_stream_.outgoing_non_data_frames.push(http::frame(http::settings_frame(settings.begin(), settings.end()), 0x0));
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    bool connection::send_countinuation(std::uint32_t stream_id, const header_block&head, bool end_headers)
    {
      bool ret = false;

      std::map<std::uint32_t,stream>::iterator it = this->streams_.find(stream_id);

      if (it == this->streams_.end())
      {
        // TODO: Handle error
      }
      else
      {
        std::string header_data;
        http::header_block::serialize(this->hpack_encoder_, head, header_data);
        if (header_data.size() > this->settings_[setting_code::max_frame_size])
        {
          // TODO: Handle error
        }
        else
        {
          it->second.outgoing_non_data_frames.push(http::frame(http::continuation_frame(header_data.data(), (std::uint32_t)header_data.size(), end_headers), stream_id));
          this->run_send_loop();
          ret = true;
        }
      }

      return ret;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    bool connection::send_data(std::uint32_t stream_id, const char *const data, std::uint32_t data_sz, bool end_stream)
    {
      bool ret = false;

      std::map<std::uint32_t,stream>::iterator it = this->streams_.find(stream_id);

      if (it != this->streams_.end())
      {
        bool state_change_result = true;
        if (end_stream)
          state_change_result = this->handle_outgoing_end_stream_state_change(it->second);

        if (state_change_result)
        {
          it->second.outgoing_data_frames.push(http::frame(http::data_frame(data, data_sz, end_stream), stream_id));
          this->run_send_loop();
          ret = true;
        }
        else
        {
          assert(!"Stream state change not allowed.");
        }
      }

      return ret;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    bool connection::send_push_promise(std::uint32_t stream_id, const header_block&head, std::uint32_t promised_stream_id, bool end_headers)
    {
      bool ret = false;

      std::map<std::uint32_t,stream>::iterator it = this->streams_.find(stream_id);

      if (it == this->streams_.end())
      {
        // TODO: Handle error
      }
      else
      {
        std::string header_data;
        http::header_block::serialize(this->hpack_encoder_, head, header_data);
        const std::uint8_t EXTRA_BYTE_LENGTH_NEEDED_FOR_HEADERS_FRAME = 0; //TODO: Set correct value
        if ((header_data.size() + EXTRA_BYTE_LENGTH_NEEDED_FOR_HEADERS_FRAME) > this->settings_[setting_code::max_frame_size])
        {
          // TODO: Handle error
        }
        else
        {
          it->second.outgoing_non_data_frames.push(http::frame(http::push_promise_frame(header_data.data(), (std::uint32_t)header_data.size(), promised_stream_id, end_headers), stream_id));
          this->run_send_loop();
          ret = true;
        }
      }

      return ret;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void connection::send_ping(std::uint64_t opaque_data)
    {
      this->root_stream_.outgoing_non_data_frames.push(http::frame(http::ping_frame(opaque_data), 0x0));
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void connection::send_ping_acknowledgement(std::uint64_t opaque_data)
    {
      this->root_stream_.outgoing_non_data_frames.push(http::frame(http::ping_frame(opaque_data, true), 0x0));
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void connection::send_goaway(http::errc error_code, const char *const data, std::uint32_t data_sz)
    {
      this->root_stream_.outgoing_non_data_frames.push(http::frame(http::goaway_frame(this->streams_.size() ? this->streams_.rbegin()->first : 0, error_code, data, data_sz), 0x0));
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    bool connection::send_window_update(std::uint32_t stream_id, std::uint32_t amount)
    {
      bool ret = false;

      std::map<std::uint32_t,stream>::iterator it = this->streams_.find(stream_id);
      if (it != this->streams_.end())
      {
        it->second.outgoing_data_frames.push(http::frame(http::window_update_frame(amount), stream_id));
        this->run_send_loop();
        ret = true;
      }

      return ret;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void connection::send_connection_level_window_update(std::uint32_t amount)
    {
      this->root_stream_.outgoing_data_frames.push(http::frame(http::window_update_frame(amount), 0x0));
      this->run_send_loop();
    }
    //----------------------------------------------------------------//

//    //----------------------------------------------------------------//
//    void connection::send(char* buf, std::size_t buf_size, const std::function<void(const std::error_code& ec, std::size_t bytes_transferred)>& handler)
//    {
//      this->socket_.async_send(asio::buffer(buf, buf_size), handler);
//    }
//    //----------------------------------------------------------------//
//
//    //----------------------------------------------------------------//
//    void connection::recv(const char* buf, std::size_t buf_size, const std::function<void(const std::error_code& ec, std::size_t bytes_transferred)>& handler)
//    {
//      this->socket_.async_send(asio::buffer(buf, buf_size), handler);
//    }
//    //----------------------------------------------------------------//
//
//    //----------------------------------------------------------------//
//    void connection::recvMessageHead()
//    {
//      std::shared_ptr<connection> self = this->shared_from_this();
//
//
//      TCP::recvline(this->socket_, this->incomingHeadBuffer_.data(), this->incomingHeadBuffer_.size(), [self](const std::error_code& ec, std::size_t bytes_transferred)
//      {
//        if (ec)
//        {
//          std::cout << ec.message() << ":" __FILE__ << "/" << __LINE__ << std::endl;
//        }
//        else
//        {
//
//          http::request_head requestHead;
//          http::header_block::deserialize(std::string(self->incomingHeadBuffer_.data(), bytes_transferred), requestHead);
//
//          std::cout << requestHead.url() << ":" __FILE__ << "/" << __LINE__ << std::endl;
//
//          this->requests_.emplace(std::make_shared<http::server::request>(requestHead, this->socket_));
//        }
//      }, "\r\n\r\n");
//    }
//    //----------------------------------------------------------------//
//
//    //----------------------------------------------------------------//
//    void connection::close()
//    {
//      this->socket_.close();
//      this->server_.httpSessions_.erase(std::shared_ptr<Session>(this));
//    }
//    //----------------------------------------------------------------//
  }
}