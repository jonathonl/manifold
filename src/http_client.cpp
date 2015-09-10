
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
      std::cout << "client::request::~request()" << std::endl;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    request_head& client::request::head()
    {
      return this->head_;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    bool client::request::send_headers(bool end_stream)
    {
      if (this->head_.method() == "get" || this->head_.method() == "head")
        end_stream = true;
      return outgoing_message::send_headers(end_stream);
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void client::request::on_push_promise(const std::function<void(http::client::request&& request)>& cb)
    {
      this->connection_->on_push_promise(this->stream_id_, [this, cb](http::header_block&& headers, std::uint32_t promised_stream_id)
      {
        cb(http::client::request(std::move(headers), this->connection_, promised_stream_id));
      });
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void client::request::on_informational_headers(const std::function<void(http::response_head&& resp_head)>& cb)
    {
      this->on_informational_headers_ = cb;
      this->connection_->on_headers(this->stream_id_, std::bind(&request::handle_on_headers, this, std::placeholders::_1));
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void client::request::on_response(const std::function<void(http::client::response&& resp)>& cb)
    {
      this->on_response_ = cb;
      this->connection_->on_headers(this->stream_id_, std::bind(&request::handle_on_headers, this, std::placeholders::_1));
    }
    //----------------------------------------------------------------//

    void client::request::handle_on_headers(http::header_block&& headers)
    {
      // TODO: need to make sure response is only received once and that all of thes happen in the correct order.
      int status_code = atoi(headers.header(":status").c_str());
      if (status_code == 0)
      {
        // assuming trailers

      }
      else if (status_code < 200)
      {
        this->on_informational_headers_ ? this->on_informational_headers_(std::move(headers)) : void();
      }
      else
      {
        this->on_response_ ? this->on_response_(client::response(std::move(headers), this->connection_, this->stream_id_)) : void();
      }
    }

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
      std::cout << "client::response::~response()" << std::endl;
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
      this->closed_ = false;
      this->next_stream_id_= 1;
      std::shared_ptr<non_tls_connection> c = std::make_shared<non_tls_connection>(ioservice);
      this->tcp_resolver_.async_resolve(asio::ip::tcp::resolver::query(host, std::to_string(port)), [this, c](const std::error_code& ec, asio::ip::tcp::resolver::iterator it)
      {
        if (ec)
        {
          this->ec_ = static_cast<std::uint32_t>(errc::internal_error);
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
              this->ec_ = static_cast<std::uint32_t>(errc::internal_error);
              this->on_close_ ? this->on_close_(this->ec_) : void();
            }
            else
            {
              asio::async_write(c->raw_socket(), asio::buffer(connection::preface.data(), connection::preface.size()), [this, c](const std::error_code& ec, std::size_t bytes_transfered)
              {
                if (ec)
                {
                  this->ec_ = static_cast<std::uint32_t>(errc::internal_error);
                  this->on_close_ ? this->on_close_(this->ec_) : void();
                }
                else
                {
                  this->connection_ = c;
                  this->connection_->run();
                  this->connection_->send_settings({});
                  this->on_connect_ ? this->on_connect_() : void();
                }
              });
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
      this->closed_ = false;
      this->next_stream_id_= 1;
      std::shared_ptr<tls_connection> c = std::make_shared<tls_connection>(ioservice, *this->ssl_context_);
      this->tcp_resolver_.async_resolve(asio::ip::tcp::resolver::query(host, std::to_string(port)), [this, c](const std::error_code& ec, asio::ip::tcp::resolver::iterator it)
      {
        if (ec)
        {
          this->ec_ = static_cast<std::uint32_t>(errc::internal_error);
          this->on_close_ ? this->on_close_(this->ec_) : void();
        }
        else
        {
          std::cout << it->host_name() << std::endl;
          std::cout << it->endpoint().port() << std::endl;
          c->socket().async_connect(*it, [this, c](const std::error_code& ec)
          {
            if (ec)
            {
              this->ec_ = static_cast<std::uint32_t>(errc::internal_error);
              this->on_close_ ? this->on_close_(this->ec_) : void();
            }
            else
            {
              c->ssl_stream().async_handshake(asio::ssl::stream_base::client, [this, c](const std::error_code& ec)
              {
                asio::async_write(c->ssl_stream(), asio::buffer(connection::preface.data(), connection::preface.size()), [this, c](const std::error_code& ec, std::size_t bytes_transfered)
                {
                  if (ec)
                  {
                    this->ec_ = static_cast<std::uint32_t>(errc::internal_error);
                    this->on_close_ ? this->on_close_(this->ec_) : void();
                  }
                  else
                  {
                    this->connection_ = c;
                    this->connection_->run();
                    this->connection_->send_settings({});
                    this->on_connect_ ? this->on_connect_() : void();
                  }
                });
              });
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

    void client::close()
    {
      if (!this->closed_)
      {
        this->closed_ = true;
        this->io_service_.post([this]()
        {
          if (this->on_close_)
            this->on_close_(this->ec_); // TODO: set with appropriate error;
          this->on_close_ = nullptr;
          this->on_connect_ = nullptr;
          this->on_push_promise_ = nullptr;
          if (this->connection_)
          {
            this->connection_->close();
            this->connection_ = nullptr;
          }
        });
      }
    }

    //----------------------------------------------------------------//
    std::uint32_t client::get_next_stream_id()
    {
      std::uint32_t ret = 0;
      if (this->next_stream_id_ <= client::max_stream_id)
      {
        ret = this->next_stream_id_;
        this->next_stream_id_ += 2;
      }
      return ret;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    http::client::request client::make_request()
    {

      //TODO: this method needs better error handling.

      if (!this->connection_)
        throw std::invalid_argument("No connection.");

      std::uint32_t next_stream_id = this->get_next_stream_id(); // could be zero;
      this->connection_->create_stream(next_stream_id);

      return client::request(http::request_head("/", "GET", {{"user-agent", this->default_user_agent_}}), this->connection_, next_stream_id);;
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
    void client::on_close(const std::function<void(std::uint32_t ec)>& fn)
    {
      bool had_previous_handler = (bool)this->on_close_;
      this->on_close_ = fn;
      // TODO: Add is_closed() to connection class.
      /*if (!had_previous_handler && this->connection_ && this->connection_.is_closed() && this->on_close_)
        this->on_close_(this->ec_);*/
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void client::set_default_user_agent(const std::string user_agent)
    {
      this->default_user_agent_ = user_agent;
    }
    //----------------------------------------------------------------//
  }
}
