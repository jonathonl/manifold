
#include "tcp.hpp"
#include "http_client.hpp"

namespace manifold
{
  namespace http
  {
    //----------------------------------------------------------------//
    client::request::request(request_head&& head, const std::shared_ptr<http::connection>& conn, std::uint32_t stream_id)
      : outgoing_message(this->head_, conn, stream_id)
    {
      this->head_ = std::move(head);
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    client::request::~request()
    {
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    const request_head& client::request::head() const
    {
      return this->head_;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void client::request::on_push_promise(const std::function<void(http::client::request&& request)>& cb)
    {
      //TODO: impl
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void client::request::on_response(const std::function<void(http::client::response&& resp)>& cb)
    {

    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    client::response::response(response_head&& head, const std::shared_ptr<http::connection>& conn, std::uint32_t stream_id)
      : incoming_message(this->head_, conn, stream_id)
    {
      this->head_ = std::move(head);
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    client::response::~response()
    {

    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    const response_head& client::response::head() const
    {
      return this->head_;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    client::client(asio::io_service& ioservice, const std::string& host, short port)
      : io_service_(ioservice), tcp_resolver_(ioservice)
    {
      this->last_stream_id_ = (std::uint32_t)-1;
      std::shared_ptr<non_tls_connection> c = std::make_shared<non_tls_connection>(ioservice);
      this->tcp_resolver_.async_resolve(asio::ip::tcp::resolver::query(host, std::to_string(port)), [this, c](const std::error_code& ec, asio::ip::tcp::resolver::iterator it)
      {
        if (ec)
        {
          this->ec_ = ec;
          this->on_close_ ? this->on_close_(this->ec_) : void();
        }
        else
        {
          c->socket().async_connect(*it, [this, c](const std::error_code& ec)
          {
            if (ec)
            {
              this->ec_ = ec;
              this->on_close_ ? this->on_close_(this->ec_) : void();
            }
            else
            {
              this->connection_ = c;
              this->on_connect_ ? this->on_connect_() : void();
            }
          });
        }
      });
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    client::client(asio::io_service& ioservice, const std::string& host, const ssl_options& options, short port)
        : io_service_(ioservice), tcp_resolver_(ioservice), ssl_context_(new asio::ssl::context(options.method))
    {
      this->last_stream_id_ = (std::uint32_t)-1;
      std::shared_ptr<tls_connection> c = std::make_shared<tls_connection>(ioservice, *this->ssl_context_);
      this->tcp_resolver_.async_resolve(asio::ip::tcp::resolver::query(host, std::to_string(port)), [this, c](const std::error_code& ec, asio::ip::tcp::resolver::iterator it)
      {
        if (ec)
        {
          this->ec_ = ec;
          this->on_close_ ? this->on_close_(this->ec_) : void();
        }
        else
        {
          c->socket().async_connect(*it, [this](const std::error_code& ec)
          {
            if (ec)
            {
              this->ec_ = ec;
              this->on_close_ ? this->on_close_(this->ec_) : void();
            }
            else
            {
              // ... TODO: SSL Handshake.
            }
          });
        }
      });
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    client::~client()
    {
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    std::uint32_t client::get_next_stream_id()
    {
      if (this->last_stream_id_ != client::max_stream_id)
      {
        this->last_stream_id_ += 2;
        return this->last_stream_id_;
      }
      return 0;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    std::uint32_t client::make_request(http::request_head&& req_head, const std::function<void(http::client::request&& req)>& cb)
    {
      std::uint32_t next_stream_id = 0;
      //TODO: this method needs better error handling.
      if (!cb)
        throw std::invalid_argument("Callback cannot be null.");

      if (this->connection_)
      {
        next_stream_id = this->get_next_stream_id();
        if (next_stream_id)
        {
          this->connection_->create_stream(next_stream_id);
          this->connection_->send_headers(next_stream_id, req_head, true, (req_head.method() == "GET" || req_head.method() == "HEAD"));
          cb(client::request(std::move(req_head), this->connection_, next_stream_id));
        }
      }
      return next_stream_id;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void client::on_connect(const std::function<void()>& cb)
    {
      bool had_previous_handler = (bool)this->on_connect_;
      this->on_connect_ = cb;
      if (!had_previous_handler && this->connection_ && this->on_connect_)
        this->on_connect_();
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void client::on_close(const std::function<void(const std::error_code ec)>& cb)
    {
      bool had_previous_handler = (bool)this->on_close_;
      this->on_close_ = cb;
      // TODO: Add is_closed() to connection class.
      /*if (!had_previous_handler && this->connection_ && this->connection_.is_closed() && this->on_close_)
        this->on_close_(this->ec_);*/
    }
    //----------------------------------------------------------------//
  }
}
