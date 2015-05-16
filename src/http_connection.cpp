
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

    //----------------------------------------------------------------//
    connection::connection()
    : hpack_encoder_(4096), hpack_decoder_(4096), stream_dependency_tree_(&this->root_stream_)
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
      this->send_loop_running_ = false;

    };
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    connection::~connection()
    {
    }
    //----------------------------------------------------------------//

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
    void connection::run_recv_loop()
    {
      std::shared_ptr<connection> self = this->shared_from_this();
      this->recv_frame(this->incoming_frame_, [self](const std::error_code& ec)
      {
        if (ec)
        {
          // TODO: Handle error.
        }
        else
        {
          if (self->incoming_frame_.stream_id())
          {
            auto it = self->streams_.find(self->incoming_frame_.stream_id());
            if (it == self->streams_.end())
            {
              if (self->incoming_frame_.is<headers_frame>() || self->incoming_frame_.is<push_promise_frame>())
              {
              }
              else
              {
                // TODO: Some other frames may be allowed after a stream is removed (eg, window_update).
              }
            }
            else
            {
              stream& current_stream = self->streams_[self->incoming_frame_.stream_id()];
              if (self->incoming_frame_.is<window_update_frame>())
              {
                current_stream.outgoing_window_size += self->incoming_frame_.window_update_frame().window_size_increment();
              }
              else if (self->incoming_frame_.is<data_frame>())
              {
                self->streams_[self->incoming_frame_.stream_id()].incoming_data_frames.push(std::move(self->incoming_frame_));
                if (current_stream.on_data_frame)
                {
                  while (current_stream.incoming_data_frames.size())
                  {
                    current_stream.on_data_frame(current_stream.incoming_data_frames.front().data_frame().data(), current_stream.incoming_data_frames.front().data_frame().data_length());

                    if (current_stream.incoming_data_frames.front().data_frame().flags() & frame_flag::end_stream)
                    {
                      current_stream.end_stream_frame_received= true;
                      if (current_stream.on_end_frame)
                        current_stream.on_end_frame();
                    }

                    current_stream.incoming_data_frames.pop();
                  }
                }
              }
              else if (self->incoming_frame_.is<headers_frame>())
              {
                if (current_stream.incoming_header_and_continuation_frames.size())
                {
                  // TODO: connection error
                }
                else
                {
                  current_stream.incoming_header_and_continuation_frames.push(std::move(self->incoming_frame_));

                  if (self->incoming_frame_.headers_frame().has_end_stream_flag())
                    current_stream.end_stream_frame_received= true;

                  if (self->incoming_frame_.headers_frame().has_end_headers_flag())
                  {
                    while (current_stream.incoming_header_and_continuation_frames.size())
                    {
                      // TODO: decompress into message head object.
                      current_stream.incoming_header_and_continuation_frames.pop();
                    }

                    // TODO: call on_headers function

                    if (current_stream.end_stream_frame_received && current_stream.on_end_frame)
                      current_stream.on_end_frame();
                  }
                }
              }
              else if (self->incoming_frame_.is<continuation_frame>())
              {
                if (!current_stream.incoming_header_and_continuation_frames.size())
                {
                  // TODO: connection error
                }
                else
                {
                  current_stream.incoming_header_and_continuation_frames.push(std::move(self->incoming_frame_));

                  if (self->incoming_frame_.continuation_frame().has_end_headers_flag())
                  {
                    while (current_stream.incoming_header_and_continuation_frames.size())
                    {
                      // TODO: decompress into message head object.
                      current_stream.incoming_header_and_continuation_frames.pop();
                    }

                    // TODO: call on_headers function

                    if (current_stream.end_stream_frame_received && current_stream.on_end_frame)
                      current_stream.on_end_frame();
                  }
                }
              }

            }

            //self->streams_[self->incoming_frame_.stream_id()].frames_.push_back(std::move(self->incoming_frame_));
          }
          else
          {
          }

          self->run_recv_loop();
        }
      });
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
    void connection::on_new_stream(const std::function<void(std::int32_t stream_id, std::list<std::pair<std::string,std::string>>&& headers, std::int32_t stream_dependency_id)>& fn)
    {
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void connection::on_close(const std::function<void()>& fn)
    {
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void connection::on_data_frame(std::uint32_t stream_id, const std::function<void(const char* const buf, std::size_t buf_size)>& fn)
    {
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void connection::on_end_frame(std::uint32_t stream_id, const std::function<void()>& fn)
    {
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void connection::on_rst_stream_frame(std::uint32_t stream_id, const std::function<void(const std::error_code& ec)>& fn)
    {
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void connection::on_push_promise(std::uint32_t stream_id, const std::function<void(http::header_block&& headers)>& fn)
    {
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void connection::on_window_update(std::uint32_t stream_id, const std::function<void()>& fn)
    {
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    bool connection::create_stream(std::uint32_t stream_id) //TODO: allow for dependency other than root.
    {
      bool ret = false;

      std::pair<std::map<std::uint32_t,stream>::iterator,bool> insert_res = this->streams_.emplace(stream_id, stream());
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
          it->second.outgoing_non_data_frames.push(http::frame(http::headers_frame(header_data.data(), (std::uint32_t)header_data.size(), end_headers, end_stream), stream_id));
          this->run_send_loop();
        }
      }

      return ret;
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
        it->second.outgoing_data_frames.push(http::frame(http::data_frame(data, data_sz, end_stream), stream_id));
        this->run_send_loop();
      }

      return ret;
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