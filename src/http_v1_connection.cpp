
#include "http_v1_connection.hpp"

#include <sstream>

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
      auto self = shared_from_this();
      this->socket_->recvline(this->recv_buffer_.data(), this->recv_buffer_.size(), [self](const std::error_code& ec, std::size_t bytes_read)
      {
        if (ec)
        {
          // TODO: handle error.
        }
        else
        {
          std::stringstream is(std::string(self->recv_buffer_.data(), bytes_read));
          v1_message_head headers;
          v1_message_head::deserialize(is, headers);

          std::string transfer_encoding = headers.header("transfer-encoding");
          std::uint64_t content_length = 0;
          std::stringstream content_length_ss(headers.header("content-length"));
          content_length_ss >> content_length;

          self->on_headers_(std::move(headers));

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
      auto self = shared_from_this();
      this->socket_->recvline(this->recv_buffer_.data(), this->recv_buffer_.size(), [self](const std::error_code& ec, std::size_t bytes_read)
      {
        if (ec)
        {
          // TODO: handle error.
        }
        else
        {
          std::stringstream is(std::string(self->recv_buffer_.data(), bytes_read));
          v1_header_block header_block;
          v1_header_block::deserialize(is, header_block);

          if (header_block.size())
          {
            self->on_trailers_ ? self->on_trailers_(std::move(header_block)) : void();
          }

          self->on_end_ ? self->on_end_() : void();

          self->run_recv_loop();
        }
      }, "\r\n\r\n");
    }

    void v1_connection::recv_chunk()
    {
      auto self = shared_from_this();
      this->socket_->recvline(this->recv_buffer_.data(), this->recv_buffer_.size(), [self](const std::error_code& ec, std::size_t bytes_read)
      {
        if (ec)
        {
          // TODO: handle error.
        }
        else
        {
          // parse chunk size.
          std::string size_line(self->recv_buffer_.data(), bytes_read);
          std::size_t pos = size_line.find(';');
          std::string chunk_size_str = size_line.substr(0, pos);
          chunk_size_str.erase(0, chunk_size_str.find_first_not_of(" \r\n"));
          chunk_size_str.erase(chunk_size_str.find_last_not_of(" \r\n") + 1);

          char* not_converted = nullptr;
          unsigned long chunk_size = strtoul(chunk_size_str.c_str(), &not_converted, 16);
          if (not_converted != nullptr)
          {
            // TODO: handle error.
          }
          else
          {
            if (chunk_size == 0)
            {
              self->recv_trailers();
            }
            else
            {
              self->socket_->recv(self->recv_buffer_.data(), chunk_size, [self, chunk_size](const std::error_code& ec, std::size_t bytes_read)
              {
                if (ec)
                {
                  // TODO: handle error.
                }
                else
                {
                  assert(chunk_size == bytes_read);
                  self->on_data_ ? self->on_data_(self->recv_buffer_.data(), bytes_read) : void();

                  self->socket_->recv(self->recv_buffer_.data(), 2, [self, chunk_size](const std::error_code& ec, std::size_t bytes_read)
                  {
                    if (ec)
                    {
                      // TODO: handle error.
                    }
                    else
                    {
                      assert(bytes_read == 2);
                      self->recv_chunk();
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

        auto self = shared_from_this();
        this->socket_->recv(this->recv_buffer_.data(), bytes_to_read, [self, content_length](const std::error_code& ec, std::size_t bytes_read)
        {
          if (ec)
          {
            // TODO: handle error.
          }
          else
          {
            self->on_data_ ? self->on_data_(self->recv_buffer_.data(), self->recv_buffer_.size()) : void();
            self->recv_known_length_body(content_length - bytes_read);
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

              auto self = shared_from_this();
              this->socket_->send(current_message.head_data.data(), current_message.head_data.size(), [self](const std::error_code &ec, std::size_t bytes_sent)
              {
                if (ec)
                {
                  // TODO: handle error.
                }
                else
                {
                  self->send_loop_running_ = false;
                  self->run_send_loop();
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
              auto self = shared_from_this();
              this->socket_->send(current_message.body.front().data(), current_message.body.front().size(), [self, &current_message](const std::error_code &ec, std::size_t bytes_sent)
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
                  self->send_loop_running_ = false;
                  self->run_send_loop();
                }
              });
            }
          }
          else if (current_message.ended)
          {
            if (current_message.trailer_data.size())
            {
              auto self = shared_from_this();
              this->socket_->send(current_message.trailer_data.data(), current_message.trailer_data.size(), [self, &current_message](const std::error_code &ec, std::size_t bytes_sent)
              {
                if (ec)
                {
                  // TODO: handle error.
                }
                else
                {
                  assert(current_message.trailer_data.size() == bytes_sent);
                  self->send_queue_.pop_front();
                  self->send_loop_running_ = false;
                  self->run_send_loop();
                }
              });
            }
            else
            {
              assert(current_message.content_length == 0);
              this->send_queue_.pop_front();
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
      this->send_queue_.emplace_back(queued_message(this->next_transaction_id_));
      return this->next_transaction_id_++;
    }

    void v1_connection::send_message_head(std::uint64_t transaction_id, const v1_message_head& head)
    {
      auto it = std::find_if(this->send_queue_.begin(), this->send_queue_.end(), [transaction_id](const queued_message& m) { return m.id == transaction_id; });
      if (it != this->send_queue_.end() && !it->head_sent)
      {
        std::stringstream content_length(head.header("content-length"));
        content_length >> it->content_length;
        std::string transfer_encoding = head.header("transfer-encoding");
        it->chunked_encoding = (transfer_encoding.size() && transfer_encoding != "identity");

        std::stringstream ss;
        v1_message_head::serialize(head, ss);
        it->head_data = std::move(ss.str());

        this->run_send_loop();
      }
    }

    void v1_connection::send_message_body(std::uint64_t transaction_id, const char*const data, std::size_t data_sz)
    {
      auto it = std::find_if(this->send_queue_.begin(), this->send_queue_.end(), [transaction_id](const queued_message& m) { return m.id == transaction_id; });
      if (it != this->send_queue_.end() && !it->ended && data_sz)
      {
        if (it->chunked_encoding)
        {
          std::stringstream ss;
          ss << std::hex << data_sz;
          std::string size_line(ss.str() + "\r\n");
          std::vector<char> tmp(size_line.size() + data_sz + 2);
          std::memcpy(tmp.data(), size_line.data(), size_line.size());
          std::memcpy(tmp.data() + size_line.size(), data, data_sz);
          std::memcpy(tmp.data() + size_line.size() + data_sz, "\r\n", 2);
          it->body.push(std::move(tmp));
        }
        else
        {
          std::vector<char> tmp(data_sz);
          std::memcpy(tmp.data(), data, data_sz);
          it->body.push(std::move(tmp));
        }

        this->run_recv_loop();
      }
    }

    void v1_connection::end_message(std::uint64_t transaction_id, const v1_header_block& trailers)
    {
      auto it = std::find_if(this->send_queue_.begin(), this->send_queue_.end(), [transaction_id](const queued_message& m) { return m.id == transaction_id; });
      if (it != this->send_queue_.end() && !it->ended)
      {
        if (it->chunked_encoding)
        {
          std::string size_line("0\r\n");
          std::stringstream ss;
          v1_header_block::serialize(trailers, ss);
          size_line.append(ss.str());
          it->trailer_data = std::move(size_line);

        }

        it->ended = true;

        this->run_send_loop();
      }
    }
  }
}

