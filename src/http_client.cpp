
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
      auto sock = std::make_shared<manifold::socket>(ioservice);
      this->tcp_resolver_.async_resolve(asio::ip::tcp::resolver::query(host, std::to_string(port)), [this, sock](const std::error_code& ec, asio::ip::tcp::resolver::iterator it)
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
          asio::async_connect(*sock, *it, [this, sock](const std::error_code& ec)
          {
            if (ec)
            {
              this->ec_ = ec;
              this->on_close_ ? this->on_close_(this->ec_) : void();
            }
            else
            {
              asio::async_write(*sock, asio::buffer(connection::preface.data(), connection::preface.size()), [this, sock](const std::error_code& ec, std::size_t bytes_transfered)
              {
                if (ec)
                {
                  this->ec_ = ec;
                  this->on_close_ ? this->on_close_(this->ec_) : void();
                }
                else
                {
                  this->connection_ = std::make_shared<http::connection>(std::move(*sock));
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
      this->last_stream_id_ = (std::uint32_t)-1;
      auto sock = std::make_shared<manifold::tls_socket>(ioservice);
      this->tcp_resolver_.async_resolve(asio::ip::tcp::resolver::query(host, std::to_string(port)), [this, sock](const std::error_code& ec, asio::ip::tcp::resolver::iterator it)
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
          asio::async_connect(sock, *it, [this, sock](const std::error_code& ec)
          {
            if (ec)
            {
              this->ec_ = ec;
              this->on_close_ ? this->on_close_(this->ec_) : void();
            }
            else
            {
              ((asio::ssl::stream<asio::ip::tcp::socket>*)sock)->async_handshake(asio::ssl::stream_base::client, [this, sock](const std::error_code& ec)
              {
                asio::async_write(sock, asio::buffer(connection::preface.data(), connection::preface.size()), [this, sock](const std::error_code& ec, std::size_t bytes_transfered)
                {
                  if (ec)
                  {
                    this->ec_ = ec;
                    this->on_close_ ? this->on_close_(this->ec_) : void();
                  }
                  else
                  {
                    this->connection_ = std::make_shared<http::connection>(std::move(*sock));
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
    void client::on_close(const std::function<void(const std::error_code ec)>& fn)
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
