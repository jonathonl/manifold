
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
    connection::connection(asio::ip::tcp::socket&& sock)
    : socket_(std::move(sock)), stream_dependency_tree_(nullptr)
    {
      std::seed_seq seed({static_cast<std::uint32_t>((std::uint64_t)this), static_cast<std::uint32_t>((std::uint64_t)&sock)});
      this->rg_.seed(seed);

      std::cout << this->socket_.non_blocking() << ":" __FILE__ << "/" << __LINE__ << std::endl;

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
    bool connection::check_tree_for_outgoing_frame(const stream_dependency_tree& current_node)
    {
      bool ret = false;

      if (current_node.stream_ptr()->outgoing_frames.size())
        ret = true;

      for (auto it = current_node.children().begin(); !ret && it != current_node.children().end(); ++it)
      {
        if (it->stream_ptr()->outgoing_frames.size())
          ret = true;
        else if (it->children().size())
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

      std::uint64_t weight_sum = 0;
      std::vector<const stream_dependency_tree*> pool;

      for (auto it = current_node.children().begin(); it != current_node.children().end(); ++it)
      {
        if (check_tree_for_outgoing_frame(*it))
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
            if (current_pool_node->stream_ptr()->outgoing_frames.size())
              ret = current_pool_node->stream_ptr();
            else
              ret = this->get_next_send_stream_ptr(*current_pool_node);
          }
        }
      }

      return ret;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void connection::run_recv_loop()
    {
      auto self = shared_from_this();
      http::frame::recv_frame(this->socket_, this->incoming_frame_, [self](const std::error_code& ec)
      {
        if (ec)
        {
          // TODO: Handle error.
        }
        else
        {
          if (self->incoming_frame_.stream_id())
          {
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
          this->outgoing_frame_ = std::move(prioritized_stream_ptr->outgoing_frames.front());
          prioritized_stream_ptr->outgoing_frames.pop();

          http::frame::send_frame(this->socket_, this->outgoing_frame_, [self](const std::error_code& ec)
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
    void connection::on_window_update(std::uint32_t stream_id, const std::function<void()>& fn)
    {
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    bool connection::send_headers(std::uint32_t stream_id, const message_head &head, bool end_stream)
    {
      return false;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    bool connection::send_data(std::uint32_t stream_id, const char *const data, std::uint32_t data_sz, bool end_stream)
    {
      bool ret = false;

      std::map<std::uint32_t,stream>::iterator it = this->streams_.find(stream_id);

      if (it != this->streams_.end())
      {
        it->second.outgoing_frames.push(http::frame(http::data_frame(data, data_sz, end_stream), stream_id));
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
//          http::message_head::deserialize(std::string(self->incomingHeadBuffer_.data(), bytes_transferred), requestHead);
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