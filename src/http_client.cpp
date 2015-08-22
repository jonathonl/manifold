
#include "tcp.hpp"
#include "http_client.hpp"

namespace manifold
{
  namespace http
  {
    //----------------------------------------------------------------//
    client::request::request(request_head&& head, const std::shared_ptr<http::connection>& conn, std::uint32_t stream_id)
      : outgoing_message(conn, stream_id)
    {
      this->head_ = std::move(head);
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    client::request::request(request&& source)
     : outgoing_message(std::move(source)), head_(std::move(source.head_))
    {
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    client::request& client::request::operator=(request&& source)
    {
      if (this != &source)
      {
        outgoing_message::operator=(std::move(source));
        this->head_ = std::move(source.head_);
      }
      return *this;
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
      this->connection_->on_push_promise(this->stream_id_, [this, cb](http::header_block&& headers)
      {
        http::client::request r(http::request_head(), this->connection_, this->stream_id_);
      });
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void client::request::on_informational_headers(const std::function<void(http::response_head&& resp_head)>& cb)
    {

    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void client::request::on_response(const std::function<void(http::client::response&& resp)>& cb)
    {

    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    client::response::response(response_head&& head, const std::shared_ptr<http::connection>& conn, std::uint32_t stream_id)
      : incoming_message(conn, stream_id)
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
    client::client(asio::io_service& ioservice, const std::string& host, unsigned short port)
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
          std::cout << it->host_name() << std::endl;
          std::cout << it->endpoint().address().to_string() << std::endl;
          std::cout << it->endpoint().port() << std::endl;
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
    client::client(asio::io_service& ioservice, const std::string& host, const ssl_options& options, unsigned short port)
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
          std::cout << it->host_name() << std::endl;
          std::cout << it->endpoint().port() << std::endl;
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
    http::client::request client::make_request(http::request_head&& req_head)
    {

      //TODO: this method needs better error handling.

      if (!this->connection_)
        throw std::invalid_argument("No connection.");

      std::uint32_t next_stream_id = this->get_next_stream_id(); // could be zero;

      this->connection_->create_stream(next_stream_id);
      std::string req_method = req_head.method();
      client::request r(std::move(req_head), this->connection_, next_stream_id);
      r.send_headers(req_method == "GET" || req_method == "HEAD");
      return r;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void client::on_connect(const std::function<void()>& fn)
    {
      bool had_previous_handler = (bool)this->on_connect_;
      this->on_connect_ = fn;
      if (!had_previous_handler && this->connection_ && this->on_connect_)
        this->on_connect_();
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void client::on_close(const std::function<void(const std::error_code ec)>& fn)
    {
      bool had_previous_handler = (bool)this->on_close_;
      this->on_close_ = fn;
      // TODO: Add is_closed() to connection class.
      /*if (!had_previous_handler && this->connection_ && this->connection_.is_closed() && this->on_close_)
        this->on_close_(this->ec_);*/
    }
    //----------------------------------------------------------------//
  }
}
