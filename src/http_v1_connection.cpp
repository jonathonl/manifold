
#include "http_v1_connection.hpp"

namespace manifold
{
  namespace http
  {
    void v1_connection::on_data(std::uint32_t stream_id, const std::function<void(const char* const buf, std::size_t buf_size)>& fn)
    {
      this->on_data_ = fn;
    }

    void v1_connection::on_headers(std::uint32_t stream_id, const std::function<bool(v1_header_block&& headers)>& fn)
    {
      this->on_headers_ = fn;
    }

    void v1_connection::on_close(std::uint32_t stream_id, const std::function<void(std::uint32_t error_code)>& fn)
    {
      this->on_close_ = fn;
    }

    void v1_connection::on_end(std::uint32_t stream_id, const std::function<void()>& fn)
    {
      this->on_end_ = fn;
    }

    void v1_connection::on_drain(std::uint32_t stream_id, const std::function<void()>& fn)
    {
      this->on_drain_ = fn;
    }

    void v1_connection::run()
    {
      this->run_recv_loop();
      this->run_send_loop();
    }

    void v1_connection::run_recv_loop()
    {
      this->socket_->recv_until(this->headers_buffer_, "\r\n\r\n", [this](const std::error_code& ec, std::size_t bytes_read)
      {
        if (ec)
        {
          // TODO: handle error.
        }
        else
        {
          std::istream is(&this->headers_buffer_);
          v1_header_block headers;
          v1_header_block::deserialize(is, headers);

          std::string transfer_encoding = headers.header("transfer-encoding");
          std::uint64_t content_length = atoll(headers.header("content-length").c_str());

          this->on_headers_(std::move(headers));

          if (transfer_encoding.empty() || transfer_encoding == "identity")
          {
          }
          else
          {

          }
        }
      });
    }

    void v1_connection::recv_known_length_body(std::uint64_t content_length)
    {

      std::size_t bytes_to_read = 4096;
      if (bytes_to_read > content_length)
        bytes_to_read = content_length;

      if (bytes_to_read < this->headers_buffer_.size())
      {

      }




    }
  }
}

