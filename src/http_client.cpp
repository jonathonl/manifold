
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
    request_head& client::request::head()
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
    client::client(asio::io_service& ioservice, const std::string& host, const ssl_options& options, short port)
        : io_service_(ioservice), tcp_resolver_(ioservice), ssl_context_(new asio::ssl::context(options.method))
    {
      this->last_stream_id_ = 0;
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

    //----------------------------------------------------------------//
    void client::make_request(const std::function<void(http::client::request&& req)>& cb)
    {
      if (!cb)
        throw std::invalid_argument("Callback cannot be null.");
      auto next_stream_id = 1; //this->get_next_stream_id();
      if (!next_stream_id)
      {
        //TODO: handle error;
      }
      else
      {
        if (this->connection_)
        {
          this->connection_->create_stream(next_stream_id);
          cb(client::request(http::request_head(), this->connection_, next_stream_id));
        }
        else
        {
          this->waiting_for_connection_queue_.emplace(http::request_head(), this->connection_, next_stream_id);
        }
      }
    }
    //----------------------------------------------------------------//
  }
}
