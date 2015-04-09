
#include <iostream>

#include "http_connection.hpp"

namespace manifold
{
  namespace http
  {
    //----------------------------------------------------------------//
    connection::connection(asio::ip::tcp::socket&& sock)
    : socket_(std::move(sock))
    {
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

        bool outgoing_frame_found = false;
        for (int i = 0; !outgoing_frame_found && i < this->stream_outgoing_process_queue_.size(); ++i)
        {
          stream& s = this->streams_[this->stream_outgoing_process_queue_.front()];
          if (s.outgoing_frames.size())
          {
            this->outgoing_frame_ = std::move(s.outgoing_frames.front());
            s.outgoing_frames.pop();

            outgoing_frame_found = true;
          }

          this->stream_outgoing_process_queue_.push(this->stream_outgoing_process_queue_.front());
          this->stream_outgoing_process_queue_.pop();
        }

        if (outgoing_frame_found)
        {
          http::frame::send_frame(this->socket_, this->outgoing_frame_, [self](const std::error_code& ec)
          {
            this->send_loop_running_ = false;
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
    void connection::on_rst_stream_frame(std::uint32_t stream_id, std::function<void(const std::error_code& ec)>& fn)
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
    bool connection::send_data(std::uint32_t stream_id, const char *const data, std::size_t data_sz, bool end_stream)
    {
      bool ret = false;

      std::map<std::uint32_t,stream>::iterator it = this->streams_.find(stream_id);

      if (it != this->streams_.end())
      {
        it->second.outgoing_frames.push(http::frame(http::data_frame()))
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