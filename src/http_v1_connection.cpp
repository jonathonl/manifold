
#include "http_v1_connection.hpp"

namespace manifold
{
  namespace http
  {
    void v1_connection::on_data(const std::function<void(const char* const, std::size_t)>& fn)
    {
      this->on_data_ = fn;
    }

    void v1_connection::on_headers(const std::function<void(v1_message_head&&)>& fn)
    {
      this->on_headers_ = fn;
    }

    void v1_connection::on_trailers(const std::function<void(v1_header_block&&)>& fn)
    {
      this->on_trailers_ = fn;
    }

    void v1_connection::on_close(const std::function<void(std::uint32_t)>& fn)
    {
      this->on_close_ = fn;
    }

    void v1_connection::on_end(const std::function<void()>& fn)
    {
      this->on_end_ = fn;
    }

    void v1_connection::on_drain(const std::function<void()>& fn)
    {
      this->on_drain_ = fn;
    }

    void v1_connection::run()
    {
      this->run_recv_loop();
    }

    void v1_connection::run_recv_loop()
    {
      this->socket_->recvline(this->recv_buffer_.data(), this->recv_buffer_.size(), [this](const std::error_code& ec, std::size_t bytes_read)
      {
        if (ec)
        {
          // TODO: handle error.
        }
        else
        {
          std::stringstream is(std::string(this->recv_buffer_.data(), bytes_read));
          v1_message_head headers;
          v1_message_head::deserialize(is, headers);

          std::string transfer_encoding = headers.header("transfer-encoding");
          std::uint64_t content_length = 0;
          std::stringstream content_length_ss(headers.header("content-length"));
          content_length_ss >> content_length;

          this->on_headers_(std::move(headers));

          if (transfer_encoding.empty() || transfer_encoding == "identity")
          {
          }
          else
          {

          }
        }
      }, "\r\n\r\n");
    }

    void v1_connection::recv_trailers()
    {
      this->socket_->recvline(this->recv_buffer_.data(), this->recv_buffer_.size(), [this](const std::error_code& ec, std::size_t bytes_read)
      {
        if (ec)
        {
          // TODO: handle error.
        }
        else
        {
          std::stringstream is(std::string(this->recv_buffer_.data(), bytes_read));
          v1_header_block header_block;
          v1_header_block::deserialize(is, header_block);

          if (header_block.size())
          {
            this->on_trailers_ ? this->on_trailers_(std::move(header_block)) : void();
          }

          this->on_end_ ? this->on_end_() : void();

          this->run_recv_loop();
        }
      }, "\r\n\r\n");
    }

    void v1_connection::recv_chunk()
    {
      this->socket_->recvline(this->recv_buffer_.data(), this->recv_buffer_.size(), [this](const std::error_code& ec, std::size_t bytes_read)
      {
        if (ec)
        {
          // TODO: handle error.
        }
        else
        {
          // parse chunk size.
          std::string size_line(this->recv_buffer_.data(), bytes_read);
          std::size_t pos = size_line.find(';');
          std::string chunk_size_str = size_line.substr(0, pos);
          chunk_size_str.erase(0, chunk_size_str.find_first_not_of(" \r\n"));
          chunk_size_str.erase(chunk_size_str.find_last_not_of(" \r\n") + 1);

          char* not_converted = nullptr;
          unsigned long chunk_size = strtoul(chunk_size_str.c_str(), &not_converted, 16);
          if ((*not_converted) != nullptr)
          {
            // TODO: handle error.
          }
          else
          {
            if (chunk_size == 0)
            {
              this->recv_trailers();
            }
            else
            {
              this->socket_->recv(this->recv_buffer_.data(), chunk_size,[this, chunk_size](const std::error_code& ec, std::size_t bytes_read)
              {
                if (ec)
                {
                  // TODO: handle error.
                }
                else
                {
                  assert(chunk_size == bytes_read);
                  this->on_data_ ? this->on_data_(this->recv_buffer_.data(), bytes_read) : void();

                  this->socket_->recv(this->recv_buffer_.data(), 2, [this, chunk_size](const std::error_code& ec, std::size_t bytes_read)
                  {
                    if (ec)
                    {
                      // TODO: handle error.
                    }
                    else
                    {
                      assert(bytes_read == 2);
                      this->recv_chunk();
                    }
                  });
                }
              });
            }
          }
        }
      });
    }

    void v1_connection::recv_known_length_body(std::uint64_t content_length)
    {
      if (content_length == 0)
      {
        this->on_end_ ? this->on_end_() : void();
        this->run_recv_loop();
      }
      else
      {
        std::size_t bytes_to_read = this->recv_buffer_.size();
        if (bytes_to_read > content_length)
          bytes_to_read = content_length;

        this->socket_->recv(this->recv_buffer_.data(), bytes_to_read, [this, content_length](const std::error_code& ec, std::size_t bytes_read)
        {
          if (ec)
          {
            // TODO: handle error.
          }
          else
          {
            this->on_data_ ? this->on_data_(this->recv_buffer_.data(), this->recv_buffer_.size()) : void();
            this->recv_known_length_body(content_length - bytes_read);
          }
        });
      }
    }

    void v1_connection::run_send_loop()
    {
      if (!this->send_loop_running_)
      {
        this->send_loop_running_ = true;

        if (this->send_queue_.size())
        {
          queued_message& current_message = this->send_queue_.front();
          if (!current_message.head_sent)
          {
            if (current_message.head_data.size())
            {

              current_message.head_sent = true;
              this->socket_->send(current_message.head_data.data(), current_message.head_data.size(), [this](const std::error_code &ec, std::size_t bytes_sent)
              {
                if (ec)
                {
                  // TODO: handle error.
                }
                else
                {
                  this->send_loop_running_ = false;
                  this->run_send_loop();
                }
              });
            }
            else
            {
              this->send_loop_running_ = false;
            }
          }
          else if (current_message.body.size())
          {
            if (!current_message.chunked_encoding && current_message.body.front().size() > current_message.content_length)
            {
              // TODO: handle error.
            }
            else
            {
              this->socket_->send(current_message.body.front().data(), current_message.body.front().size(), [this, &current_message](const std::error_code &ec, std::size_t bytes_sent)
              {
                if (ec)
                {
                  // TODO: handle error.
                }
                else
                {
                  assert(current_message.body.front().size() == bytes_sent);
                  current_message.body.pop();
                  current_message.content_length -= bytes_sent;
                  this->send_loop_running_ = false;
                  this->run_send_loop();
                }
              });
            }
          }
          else if (current_message.ended)
          {
            if (current_message.trailer_data.size())
            {
              this->socket_->send(current_message.trailer_data.data(), current_message.trailer_data.size(), [this, &current_message](const std::error_code &ec, std::size_t bytes_sent)
              {
                if (ec)
                {
                  // TODO: handle error.
                }
                else
                {
                  assert(current_message.trailer_data.size() == bytes_sent);
                  this->send_queue_.pop();
                  this->send_loop_running_ = false;
                  this->run_send_loop();
                }
              });
            }
            else
            {
              assert(current_message.content_length == 0);
              this->send_queue_.pop();
              this->send_loop_running_ = false;
              this->run_send_loop();
            }
          }
          else
          {
            this->send_loop_running_ = false;
          }
        }
      }
    }

    std::uint64_t v1_connection::start_message()
    {
      this->send_queue_.push(queued_message(this->next_transaction_id_));
      return this->next_transaction_id_++;
    }
  }
}

