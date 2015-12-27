
#include "tcp.hpp"
#include "http_client.hpp"

namespace manifold
{
  namespace http
  {
    bool verify_certificate(bool preverified,
      asio::ssl::verify_context& ctx)
    {
      // The verify callback can be used to check whether the certificate that is
      // being presented is valid for the peer. For example, RFC 2818 describes
      // the steps involved in doing this for HTTPS. Consult the OpenSSL
      // documentation for more details. Note that the callback is called once
      // for each certificate in the certificate chain, starting from the root
      // certificate authority.

      // In this example we will simply print the certificate's subject name.
      std::cout << "HERE I AM!!!!!!!!!!!!!!!" << std::endl;
      char subject_name[256];
      X509* cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
      X509_NAME_oneline(X509_get_subject_name(cert), subject_name, 256);
      std::cout << "Verifying " << subject_name << "\n";

      return preverified;
    }

    int client_alpn_select_proto_cb(SSL *ssl, const unsigned char **out,
      unsigned char *out_len, const unsigned char *in,
      unsigned int in_len, void *arg)
    {
      static const char*const h2_proto_string = "\x02h2";
      std::size_t h2_proto_string_len = ::strlen(h2_proto_string);
      const unsigned char* client_proto = in;
      const unsigned char* client_proto_end = in + in_len;
      for ( ; client_proto + h2_proto_string_len <= client_proto_end; client_proto += *client_proto + 1)
      {
        std::size_t client_proto_len = (*client_proto + 1);
        if (::memcmp(h2_proto_string, client_proto, h2_proto_string_len <  client_proto_len ? h2_proto_string_len : client_proto_len) == 0)
        {
          *out = client_proto;
          *out_len = (unsigned char)client_proto_len;
          return SSL_TLSEXT_ERR_OK;
        }
      }
      return SSL_TLSEXT_ERR_NOACK;
    }

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
      this->next_stream_id_ = 1;

      std::vector<unsigned char> proto_list(::strlen(MANIFOLD_HTTP_ALPN_SUPPORTED_PROTOCOL));
      std::copy_n(MANIFOLD_HTTP_ALPN_SUPPORTED_PROTOCOL, ::strlen(MANIFOLD_HTTP_ALPN_SUPPORTED_PROTOCOL), proto_list.begin());
      //SSL_CTX_set_alpn_select_cb(this->ssl_context_->impl(), client_alpn_select_proto_cb, nullptr);
      const unsigned char* test = this->ssl_context_->impl()->alpn_client_proto_list;
      auto r = SSL_CTX_set_alpn_protos(this->ssl_context_->impl(), proto_list.data(), proto_list.size());
      const unsigned char* test2 = this->ssl_context_->impl()->alpn_client_proto_list;
      auto sock = std::make_shared<manifold::tls_socket>(ioservice, *this->ssl_context_);
      std::error_code e;
      ((asio::ssl::stream<asio::ip::tcp::socket>&)*sock).set_verify_mode(asio::ssl::verify_none, e);
      ((asio::ssl::stream<asio::ip::tcp::socket>&)*sock).set_verify_callback(verify_certificate, e);

      this->ssl_context_->set_default_verify_paths();

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
              std::cout << "ERROR: " << ec.message() << std::endl;
              this->ec_ = static_cast<std::uint32_t>(errc::internal_error);
              this->on_close_ ? this->on_close_(this->ec_) : void();
            }
            else
            {
              ((asio::ssl::stream<asio::ip::tcp::socket>&)*sock).async_handshake(asio::ssl::stream_base::client, [this, sock](const std::error_code& ec)
              {
                const unsigned char* selected_alpn = nullptr;
                unsigned int selected_alpn_sz = 1;
                //SSL_get0_alpn_selected(((asio::ssl::stream<asio::ip::tcp::socket>&)*sock).native_handle(), &selected_alpn, &selected_alpn_sz);
                if (ec)
                {
                  std::cout << "ERROR: " << ec.message() << std::endl;
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
