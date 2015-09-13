
#include "tcp.hpp"
#include "http_client.hpp"

namespace manifold
{
  namespace http
  {
    //----------------------------------------------------------------//
    void client::connection::on_informational_headers(std::uint32_t stream_id, const std::function<void(http::response_head&& headers)>& fn)
    {
      auto it = this->streams_.find(stream_id);
      if (it == this->streams_.end())
      {
        // TODO: Handle error
      }
      else
      {
        ((stream*)it->second.get())->on_informational_headers(fn);
      }
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void client::connection::on_response(std::uint32_t stream_id, const std::function<void(http::client::response&& resp)>& fn)
    {
      auto self = shared_from_this();
      auto it = this->streams_.find(stream_id);
      if (it == this->streams_.end())
      {
        // TODO: Handle error
      }
      else
      {
        ((stream*)it->second.get())->on_response_headers([self, fn, stream_id](http::response_head&& headers)
        {
          fn(http::client::response(std::move(headers), self, stream_id));
        });
      }
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void client::connection::on_trailers(std::uint32_t stream_id, const std::function<void(http::header_block&& headers)>& fn)
    {
      auto it = this->streams_.find(stream_id);
      if (it == this->streams_.end())
      {
        // TODO: Handle error
      }
      else
      {
        ((stream*)it->second.get())->on_trailers(fn);
      }
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void client::connection::on_push_promise(std::uint32_t stream_id, const std::function<void(http::client::request&& req)>& fn)
    {
      auto self = this->shared_from_this();
      auto it = this->streams_.find(stream_id);
      if (it == this->streams_.end())
      {
      // TODO: Handle error
      }
      else
      {
        it->second->on_push_promise([self, fn](http::request_head&& headers, std::uint32_t promised_stream_id)
        {
          if (!self->create_stream(promised_stream_id))
          {
            // TODO: Handle error
          }
          else
          {
            fn(http::client::request(std::move(headers), self, promised_stream_id));
          }
        });
      }
    }

    //----------------------------------------------------------------//
    client::request::request(request_head&& head, const std::shared_ptr<http::connection>& conn, std::uint32_t stream_id)
      : outgoing_message(conn, stream_id), head_(std::move(head))
    {
      ((connection*)conn.get())->on_push_promise(stream_id, [stream_id, conn](http::client::request&& req)
      {
        conn->send_reset_stream(stream_id, errc::refused_stream);
      });
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
      auto c = this->connection_;
      ((client::connection*)this->connection_.get())->on_push_promise(this->stream_id_, cb);
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void client::request::on_informational_headers(const std::function<void(http::response_head&& resp_head)>& cb)
    {
      ((client::connection*)this->connection_.get())->on_informational_headers(this->stream_id_, cb) ;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void client::request::on_response(const std::function<void(http::client::response&& resp)>& cb)
    {
      auto c = this->connection_;
      std::uint32_t stream_id = this->stream_id_;
      ((client::connection*)this->connection_.get())->on_response(this->stream_id_, cb);
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
      auto sock = std::make_shared<manifold::non_tls_socket>(ioservice);
      this->tcp_resolver_.async_resolve(asio::ip::tcp::resolver::query(host, std::to_string(port)), [this, sock](const std::error_code& ec, asio::ip::tcp::resolver::iterator it)
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
          ((asio::ip::tcp::socket&)*sock).async_connect(*it, [this, sock](const std::error_code& ec)
          {
            if (ec)
            {
              this->ec_ = static_cast<std::uint32_t>(errc::internal_error);
              this->on_close_ ? this->on_close_(this->ec_) : void();
            }
            else
            {
              sock->send(connection::preface.data(), connection::preface.size(), [this, sock](const std::error_code& ec, std::size_t bytes_transfered)
              {
                if (ec)
                {
                  this->ec_ = static_cast<std::uint32_t>(errc::internal_error);
                  this->on_close_ ? this->on_close_(this->ec_) : void();
                }
                else
                {
                  this->connection_ = std::make_shared<connection>(std::move(*sock));
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
      
      auto sock = std::make_shared<manifold::tls_socket>(ioservice, *this->ssl_context_);
      this->tcp_resolver_.async_resolve(asio::ip::tcp::resolver::query(host, std::to_string(port)), [this, sock](const std::error_code& ec, asio::ip::tcp::resolver::iterator it)
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
          ((asio::ssl::stream<asio::ip::tcp::socket>&)*sock).next_layer().async_connect(*it, [this, sock](const std::error_code& ec)
          {
            if (ec)
            {
              this->ec_ = static_cast<std::uint32_t>(errc::internal_error);
              this->on_close_ ? this->on_close_(this->ec_) : void();
            }
            else
            {
              ((asio::ssl::stream<asio::ip::tcp::socket>&)*sock).async_handshake(asio::ssl::stream_base::client, [this, sock](const std::error_code& ec)
              {
                sock->send(connection::preface.data(), connection::preface.size(), [this, sock](const std::error_code& ec, std::size_t bytes_transfered)
                {
                  if (ec)
                  {
                    this->ec_ = static_cast<std::uint32_t>(errc::internal_error);
                    this->on_close_ ? this->on_close_(this->ec_) : void();
                  }
                  else
                  {
                    this->connection_ = std::make_shared<connection>(std::move(*sock));
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
