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
      : io_service_(ioservice), tcp_resolver_(ioservice), socket_(ioservice)
    {
      this->last_stream_id_ = 0;
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
          c->socket().async_connect(*it, [this](const std::error_code& ec)
          {
            if (ec)
            {
              this->ec_ = ec;
              this->on_close_ ? this->on_close_(this->ec_) : void();
            }
            else
            {
              // ...
            }
          });
        }
      });
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    client::client(asio::io_service& ioservice, const std::string& host, const tls_options& options, short port)
        : io_service_(ioservice), tcp_resolver_(ioservice), socket_(ioservice)
    {
      this->last_stream_id_ = 0;
      std::shared_ptr<tls_connection> c = std::make_shared<tls_connection>(ioservice, options.context);
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
              // ...
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
  }
}
