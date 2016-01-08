
#include "http_v1_connection.hpp"
#include "http_v1_request_head.hpp"
#include "http_v1_response_head.hpp"

#include <sstream>

namespace manifold
{
  namespace http
  {
    template <typename SendMsg, typename RecvMsg>
    void v1_connection<SendMsg, RecvMsg>::on_close(const std::function<void(std::uint32_t error_code)>& fn)
    {
      this->on_close_ = fn;
    }

    template <typename SendMsg, typename RecvMsg>
    void v1_connection<SendMsg, RecvMsg>::on_new_stream(const std::function<void(std::uint32_t transaction_id)>& fn)
    {
      this->on_new_stream_ = fn;
    }

    template <typename SendMsg, typename RecvMsg>
    void v1_connection<SendMsg, RecvMsg>::on_data(std::uint32_t transaction_id, const std::function<void(const char* const, std::size_t)>& fn)
    {
      auto it = std::find_if(this->recv_queue_.begin(), this->recv_queue_.end(), [transaction_id](const queued_recv_message& m) { return m.id == transaction_id; });
      if (it != this->recv_queue_.end())
      {
        it->on_data = fn;
      }
    }

    template <typename SendMsg, typename RecvMsg>
    void v1_connection<SendMsg, RecvMsg>::on_headers(std::uint32_t transaction_id, const std::function<void(RecvMsg&&)>& fn)
    {
      auto it = std::find_if(this->recv_queue_.begin(), this->recv_queue_.end(), [transaction_id](const queued_recv_message& m) { return m.id == transaction_id; });
      if (it != this->recv_queue_.end())
      {
        it->on_headers = fn;
      }
    }

    template <typename SendMsg, typename RecvMsg>
    void v1_connection<SendMsg, RecvMsg>::on_informational_headers(std::uint32_t transaction_id, const std::function<void(RecvMsg&& headers)>& fn)
    {
      auto it = std::find_if(this->recv_queue_.begin(), this->recv_queue_.end(), [transaction_id](const queued_recv_message& m) { return m.id == transaction_id; });
      if (it != this->recv_queue_.end())
      {
        it->on_informational_headers = fn;
      }
    }

    template <typename SendMsg, typename RecvMsg>
    void v1_connection<SendMsg, RecvMsg>::on_trailers(std::uint32_t transaction_id, const std::function<void(header_block&&)>& fn)
    {
      auto it = std::find_if(this->recv_queue_.begin(), this->recv_queue_.end(), [transaction_id](const queued_recv_message& m) { return m.id == transaction_id; });
      if (it != this->recv_queue_.end())
      {
        it->on_trailers = fn;
      }
    }

    template <typename SendMsg, typename RecvMsg>
    void v1_connection<SendMsg, RecvMsg>::on_close(std::uint32_t transaction_id, const std::function<void(std::uint32_t)>& fn)
    {
      auto it = std::find_if(this->transaction_close_queue_.begin(), this->transaction_close_queue_.end(), [transaction_id](const queued_close_callback& m) { return m.id == transaction_id; });
      if (it != this->transaction_close_queue_.end())
      {
        it->on_close = fn;
      }
    }

    template <typename SendMsg, typename RecvMsg>
    void v1_connection<SendMsg, RecvMsg>::on_end(std::uint32_t transaction_id, const std::function<void()>& fn)
    {
      auto it = std::find_if(this->recv_queue_.begin(), this->recv_queue_.end(), [transaction_id](const queued_recv_message& m) { return m.id == transaction_id; });
      if (it != this->recv_queue_.end())
      {
        it->on_end = fn;
      }
    }

    template <typename SendMsg, typename RecvMsg>
    void v1_connection<SendMsg, RecvMsg>::on_drain(std::uint32_t transaction_id, const std::function<void()>& fn)
    {
      auto it = std::find_if(this->recv_queue_.begin(), this->recv_queue_.end(), [transaction_id](const queued_recv_message& m) { return m.id == transaction_id; });
      if (it != this->recv_queue_.end())
      {
        it->on_drain = fn;
      }
    }

    template <typename SendMsg, typename RecvMsg>
    void v1_connection<SendMsg, RecvMsg>::run()
    {
      this->run_recv_loop();
    }

    template <typename SendMsg, typename RecvMsg>
    void v1_connection<SendMsg, RecvMsg>::close(std::uint32_t ec)
    {
      if (!this->closed_)
      {
        this->closed_ = true;

        this->socket_->close();
        auto self = v1_connection<SendMsg, RecvMsg>::shared_from_this();
        this->socket_->io_service().post([self, ec]()
        {
          self->send_queue_.clear();
          self->recv_queue_.clear();

          while (self->transaction_close_queue_.size())
          {
            self->transaction_close_queue_.front().on_close ? self->transaction_close_queue_.front().on_close(ec) : void();
            self->transaction_close_queue_.pop_front();
          }

          self->on_close_ ? self->on_close_(ec) : void();
          self->on_new_stream_ = nullptr;
          self->on_close_ = nullptr;
        });
      }
    }

    template <typename SendMsg, typename RecvMsg>
    void v1_connection<SendMsg, RecvMsg>::run_recv_loop()
    {
      auto self = v1_connection<SendMsg, RecvMsg>::shared_from_this();
      if (this->recv_queue_.empty())
      {
        std::uint32_t transaction_id = this->create_stream(0, 0);
        this->on_new_stream_ ? this->on_new_stream_(transaction_id) : void();
      }

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

          self->recv_queue_.front().on_headers(std::move(headers));

          if (transfer_encoding.empty() || transfer_encoding == "identity")
          {
            self->recv_known_length_body(content_length);
          }
          else
          {
            self->recv_chunk_encoded_body();
          }
        }
      }, "\r\n\r\n");
    }

    template <typename SendMsg, typename RecvMsg>
    void v1_connection<SendMsg, RecvMsg>::recv_trailers()
    {
      auto self = v1_connection<SendMsg, RecvMsg>::shared_from_this();
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
            self->recv_queue_.front().on_trailers ? self->recv_queue_.front().on_trailers(std::move(header_block)) : void();
          }

          self->recv_queue_.front().on_end ? self->recv_queue_.front().on_end() : void();
          auto front_transaction_id = self->recv_queue_.front().id;
          if (std::find_if(self->send_queue_.begin(), self->send_queue_.end(), [front_transaction_id](const queued_send_message& m) { return (m.id == front_transaction_id); }) == self->send_queue_.end())
          {
            assert(self->transaction_close_queue_.front().id == front_transaction_id);
            self->transaction_close_queue_.front().on_close ? self->transaction_close_queue_.front().on_close(0) : void();
            self->transaction_close_queue_.pop_front();
          }
          self->recv_queue_.pop_front();

          self->run_recv_loop();
        }
      }, "\r\n\r\n");
    }

    template <typename SendMsg, typename RecvMsg>
    void v1_connection<SendMsg, RecvMsg>::recv_chunk_encoded_body()
    {
      auto self = v1_connection<SendMsg, RecvMsg>::shared_from_this();
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
                  self->recv_queue_.front().on_data ? self->recv_queue_.front().on_data(self->recv_buffer_.data(), bytes_read) : void();

                  self->socket_->recv(self->recv_buffer_.data(), 2, [self, chunk_size](const std::error_code& ec, std::size_t bytes_read)
                  {
                    if (ec)
                    {
                      // TODO: handle error.
                    }
                    else
                    {
                      assert(bytes_read == 2);
                      self->recv_chunk_encoded_body();
                    }
                  });
                }
              });
            }
          }
        }
      });
    }

    template <typename SendMsg, typename RecvMsg>
    void v1_connection<SendMsg, RecvMsg>::recv_known_length_body(std::uint64_t content_length)
    {
      if (content_length == 0)
      {
        this->recv_queue_.front().on_end ? this->recv_queue_.front().on_end() : void();
        auto front_transaction_id = this->recv_queue_.front().id;
        if (std::find_if(this->send_queue_.begin(), this->send_queue_.end(), [front_transaction_id](const queued_send_message& m) { return (m.id == front_transaction_id); }) == this->send_queue_.end())
        {
          assert(this->transaction_close_queue_.front().id == front_transaction_id);
          this->transaction_close_queue_.front().on_close ? this->transaction_close_queue_.front().on_close(0) : void();
          this->transaction_close_queue_.pop_front();
        }
        this->recv_queue_.pop_front();

        this->run_recv_loop();
      }
      else
      {
        std::size_t bytes_to_read = this->recv_buffer_.size();
        if (bytes_to_read > content_length)
          bytes_to_read = content_length;

        auto self = v1_connection<SendMsg, RecvMsg>::shared_from_this();
        this->socket_->recv(this->recv_buffer_.data(), bytes_to_read, [self, content_length](const std::error_code& ec, std::size_t bytes_read)
        {
          if (ec)
          {
            // TODO: handle error.
          }
          else
          {
            self->recv_queue_.front().on_data ? self->recv_queue_.front().on_data(self->recv_buffer_.data(), self->recv_buffer_.size()) : void();
            self->recv_known_length_body(content_length - bytes_read);
          }
        });
      }
    }

    template <typename SendMsg, typename RecvMsg>
    void v1_connection<SendMsg, RecvMsg>::run_send_loop()
    {
      if (!this->send_loop_running_)
      {
        this->send_loop_running_ = true;

        if (this->send_queue_.size())
        {
          queued_send_message& current_message = this->send_queue_.front();
          if (!current_message.head_sent)
          {
            if (current_message.head_data.size())
            {
              current_message.head_sent = true;

              auto self = v1_connection<SendMsg, RecvMsg>::shared_from_this();
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
              auto self = v1_connection<SendMsg, RecvMsg>::shared_from_this();
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
              auto self = v1_connection<SendMsg, RecvMsg>::shared_from_this();
              this->socket_->send(current_message.trailer_data.data(), current_message.trailer_data.size(), [self, &current_message](const std::error_code &ec, std::size_t bytes_sent)
              {
                if (ec)
                {
                  // TODO: handle error.
                }
                else
                {
                  assert(current_message.trailer_data.size() == bytes_sent);
                  auto front_transaction_id = self->send_queue_.front().id;
                  if (std::find_if(self->recv_queue_.begin(), self->recv_queue_.end(), [front_transaction_id](const queued_recv_message& m) { return (m.id == front_transaction_id); }) == self->recv_queue_.end())
                  {
                    assert(self->transaction_close_queue_.front().id == front_transaction_id);
                    self->transaction_close_queue_.front().on_close ? self->transaction_close_queue_.front().on_close(0) : void();
                    self->transaction_close_queue_.pop_front();
                  }
                  self->send_queue_.pop_front();
                  self->send_loop_running_ = false;
                  self->run_send_loop();
                }
              });
            }
            else
            {
              assert(current_message.content_length == 0);
              auto self = v1_connection<SendMsg, RecvMsg>::shared_from_this();
              this->socket_->io_service().post([self]()
              {
                auto front_transaction_id = self->send_queue_.front().id;
                if (std::find_if(self->recv_queue_.begin(), self->recv_queue_.end(), [front_transaction_id](const queued_recv_message& m) { return (m.id == front_transaction_id); }) == self->recv_queue_.end())
                {
                  assert(self->transaction_close_queue_.front().id == front_transaction_id);
                  self->transaction_close_queue_.front().on_close ? self->transaction_close_queue_.front().on_close(0) : void();
                  self->transaction_close_queue_.pop_front();
                }
                self->send_queue_.pop_front();

                self->send_loop_running_ = false;
                self->run_send_loop();
              });
            }
          }
          else
          {
            this->send_loop_running_ = false;
          }
        }
      }
    }

    template <typename SendMsg, typename RecvMsg>
    std::uint32_t v1_connection<SendMsg, RecvMsg>::create_stream(std::uint32_t dependency_transaction_id, std::uint32_t transaction_id)
    {
      this->send_queue_.emplace_back(queued_send_message(this->next_transaction_id_));
      this->recv_queue_.emplace_back(queued_recv_message(this->next_transaction_id_));
      return this->next_transaction_id_++;
    }

    template <typename SendMsg, typename RecvMsg>
    bool v1_connection<SendMsg, RecvMsg>::send_message_head(std::uint64_t transaction_id, const v1_message_head& head)
    {
      bool ret = false;

      auto it = std::find_if(this->send_queue_.begin(), this->send_queue_.end(), [transaction_id](const queued_send_message& m) { return m.id == transaction_id; });
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
        ret = true;
      }

      return ret;
    }

    template <>
    bool v1_connection<request_head, response_head>::send_headers(std::uint32_t stream_id, const request_head& head, bool end_headers, bool end_stream)
    {
      bool ret = false;

      v1_request_head v1_head(head);
      if (this->send_message_head(stream_id, v1_head))
      {
        if (end_stream)
          ret = this->end_message(stream_id);
        else
          ret = true;
      }

      return ret;
    }

    template <>
    bool v1_connection<response_head, request_head>::send_headers(std::uint32_t stream_id, const response_head& head, bool end_headers, bool end_stream)
    {
      bool ret = false;

      v1_response_head v1_head(head);
      if (this->send_message_head(stream_id, v1_head))
      {
        if (end_stream)
          ret = this->end_message(stream_id);
        else
          ret = true;
      }

      return ret;
    }

    template <typename SendMsg, typename RecvMsg>
    bool v1_connection<SendMsg, RecvMsg>::send_trailers(std::uint32_t stream_id, const header_block& head, bool end_headers, bool end_stream)
    {
      return this->end_message(stream_id, v1_header_block(head));
    }

    template <typename SendMsg, typename RecvMsg>
    bool v1_connection<SendMsg, RecvMsg>::send_data(std::uint32_t transaction_id, const char*const data, std::uint32_t data_sz, bool end_stream)
    {
      bool ret = false;

      auto it = std::find_if(this->send_queue_.begin(), this->send_queue_.end(), [transaction_id](const queued_send_message& m) { return m.id == transaction_id; });
      if (it != this->send_queue_.end() && !it->ended && data_sz)
      {
        if (data_sz)
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
        }

        if (end_stream)
          ret = this->end_message(transaction_id);
        else
        {
          this->run_send_loop();
          ret = true;
        }
      }

      return ret;
    }

    template <typename SendMsg, typename RecvMsg>
    bool v1_connection<SendMsg, RecvMsg>::end_message(std::uint32_t transaction_id, const v1_header_block& trailers)
    {
      bool ret = false;

      auto it = std::find_if(this->send_queue_.begin(), this->send_queue_.end(), [transaction_id](const queued_send_message& m) { return m.id == transaction_id; });
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
        ret = true;
      }

      return ret;
    }

    template class v1_connection<request_head, response_head>;
    template class v1_connection<response_head, request_head>;
  }
}

