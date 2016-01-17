
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

    class client_impl : public std::enable_shared_from_this<client_impl>
    {
    private:
      asio::io_service& io_service_;
      asio::ip::tcp::resolver tcp_resolver_;
      std::string default_user_agent_ = "Manifold";
      std::string socket_address_;

      std::unique_ptr<asio::ssl::context> ssl_context_;
      std::shared_ptr<http::connection<request_head, response_head>> connection_;
      // std::queue<std::pair<client::request, std::function<void(http::client::request && req)>>> pending_requests_;

      std::function<void()> on_connect_;
      std::function<void(errc ec)> on_close_;
      errc ec_;
      bool closed_ = false;

      void destroy_callbacks_later()
      {
        auto self = shared_from_this();
        this->io_service_.post([self]()
        {
          self->on_connect_ = nullptr;
          self->on_close_ = nullptr;
        });
      }
      //void send_connection_preface(std::function<void(const std::error_code& ec)>& fn);
    public:
      client_impl(asio::io_service& ioservice)
        : io_service_(ioservice), tcp_resolver_(ioservice)
      {

      }

      ~client_impl()
      {
        this->close(errc::cancel);
      }

      void connect(const std::string& host, unsigned short port)
      {
        if (!port)
          port = 80;

        if (port != 80)
          this->socket_address_ = host + std::to_string(port);
        else
          this->socket_address_ = host;

        auto sock = std::make_shared<manifold::non_tls_socket>(this->io_service_);
        auto self = shared_from_this();
        this->tcp_resolver_.async_resolve(asio::ip::tcp::resolver::query(host, std::to_string(port)), [self, sock](const std::error_code& ec, asio::ip::tcp::resolver::iterator it)
        {
          if (ec)
          {
            self->ec_ = errc::internal_error;
            self->on_close_ ? self->on_close_(self->ec_) : void();
          }
          else
          {
            std::cout << it->host_name() << std::endl;
            std::cout << it->endpoint().address().to_string() << std::endl;
            std::cout << it->endpoint().port() << std::endl;
            ((asio::ip::tcp::socket&)*sock).async_connect(*it, [self, sock](const std::error_code& ec)
            {
              if (ec)
              {
                self->ec_ = errc::internal_error;
                self->on_close_ ? self->on_close_(self->ec_) : void();
              }
              else
              {
                self->connection_ = std::make_shared<v1_connection<request_head, response_head>>(std::move(*sock));
                self->connection_->on_close(std::bind(&client_impl::close, self, std::placeholders::_1));
                self->connection_->run();
                self->on_connect_ ? self->on_connect_() : void();
              }
            });
          }
        });
      }

      void connect(const std::string& host, const client::ssl_options& options, unsigned short port)
      {
        this->ssl_context_ = std::unique_ptr<asio::ssl::context>(new asio::ssl::context(options.method));

        if (!port)
          port = 443;

        if (port != 443)
          this->socket_address_ = host + std::to_string(port);
        else
          this->socket_address_ = host;

        std::vector<unsigned char> proto_list(::strlen(MANIFOLD_HTTP_ALPN_SUPPORTED_PROTOCOLS));
        std::copy_n(MANIFOLD_HTTP_ALPN_SUPPORTED_PROTOCOLS, ::strlen(MANIFOLD_HTTP_ALPN_SUPPORTED_PROTOCOLS), proto_list.begin());
        //SSL_CTX_set_alpn_select_cb(this->ssl_context_->impl(), client_alpn_select_proto_cb, nullptr);
        const unsigned char* test = this->ssl_context_->impl()->alpn_client_proto_list;
        auto r = SSL_CTX_set_alpn_protos(this->ssl_context_->impl(), proto_list.data(), proto_list.size());
        const unsigned char* test2 = this->ssl_context_->impl()->alpn_client_proto_list;
        auto sock = std::make_shared<manifold::tls_socket>(this->io_service_, *this->ssl_context_);
        std::error_code e;
        ((asio::ssl::stream<asio::ip::tcp::socket>&)*sock).set_verify_mode(asio::ssl::verify_none, e);
        //((asio::ssl::stream<asio::ip::tcp::socket>&)*sock).set_verify_callback(verify_certificate, e);

        this->ssl_context_->set_default_verify_paths();

        auto self = shared_from_this();
        this->tcp_resolver_.async_resolve(asio::ip::tcp::resolver::query(host, std::to_string(port)), [self, sock](const std::error_code& ec, asio::ip::tcp::resolver::iterator it)
        {
          if (ec)
          {
            self->ec_ = errc::internal_error;
            self->on_close_ ? self->on_close_(self->ec_) : void();
          }
          else
          {
            std::cout << it->host_name() << std::endl;
            std::cout << it->endpoint().port() << std::endl;
            ((asio::ssl::stream<asio::ip::tcp::socket>&)*sock).next_layer().async_connect(*it, [self, sock](const std::error_code& ec)
            {
              if (ec)
              {
                std::cout << "ERROR: " << ec.message() << std::endl;
                self->ec_ = errc::internal_error;
                self->on_close_ ? self->on_close_(self->ec_) : void();
              }
              else
              {
                ((asio::ssl::stream<asio::ip::tcp::socket>&)*sock).async_handshake(asio::ssl::stream_base::client, [self, sock](const std::error_code& ec)
                {
                  const unsigned char* selected_alpn = nullptr;
                  unsigned int selected_alpn_sz = 0;
                  SSL_get0_alpn_selected(((asio::ssl::stream<asio::ip::tcp::socket>&)*sock).native_handle(), &selected_alpn, &selected_alpn_sz);
                  std::cout << "Client ALPN: " << std::string((char*)selected_alpn, selected_alpn_sz) << std::endl;
                  if (ec)
                  {
                    std::cout << "ERROR: " << ec.message() << std::endl;
                    self->ec_ = errc::internal_error;
                    self->on_close_ ? self->on_close_(self->ec_) : void();
                  }
#ifndef MANIFOLD_DISABLE_HTTP2
                    else if (std::string((char*)selected_alpn, selected_alpn_sz) == "h2")
                {
                  sock->send(v2_connection::preface.data(), v2_connection::preface.size(), [this, sock](const std::error_code& ec, std::size_t bytes_transfered)
                  {
                    if (ec)
                    {
                      this->ec_ = errc::internal_error;
                      this->on_close_ ? this->on_close_(this->ec_) : void();
                    }
                    else
                    {
                      this->connection_ = std::make_shared<v2_connection>(std::move(*sock));
                      this->connection_->on_close([this](errc ec) { this->on_close_ ? this->on_close_(ec) : void(); });
                      this->connection_->run();
                      this->on_connect_ ? this->on_connect_() : void();
                    }
                  });
                }
#endif //MANIFOLD_DISABLE_HTTP2
                  else
                  {
                    self->connection_ = std::make_shared<v1_connection<request_head, response_head>>(std::move(*sock));
                    self->connection_->on_close([self](errc ec) { self->on_close_ ? self->on_close_(ec) : void(); });
                    self->connection_->run();
                    self->on_connect_ ? self->on_connect_() : void();
                  }
                });
              }
            });
          }
        });
      }

      void on_connect(const std::function<void()>& fn)
      {
        if (this->connection_)
          fn ? fn() : void();
        else
          this->on_connect_ = fn;
      }

      void on_close(const std::function<void(errc ec)>& fn)
      {
        if (this->closed_)
          fn ? fn(this->ec_) : void();
        else
          this->on_close_ = fn;
      }

      client::request make_request()
      {
        //TODO: this method needs better error handling.

        if (!this->connection_)
          throw std::invalid_argument("No connection.");


        std::uint32_t stream_id = this->connection_->create_stream(0, 0);

        return client::request(request_head("/", "GET", {{"user-agent", this->default_user_agent_}}), this->connection_, stream_id, this->socket_address_);
      }

      void close(errc ec)
      {
        if (!this->closed_)
        {
          this->closed_ = true;


          if (this->connection_)
          {
            this->connection_->close(ec);
            this->connection_ = nullptr;
          }
          else
          {
            this->on_close_ ? this->on_close_(ec) : void();
          }

          this->destroy_callbacks_later();
        }
      }

      void set_default_user_agent(const std::string user_agent)
      {
        this->default_user_agent_ = user_agent;
      }
    };

//    int client_alpn_select_proto_cb(SSL *ssl, const unsigned char **out,
//      unsigned char *out_len, const unsigned char *in,
//      unsigned int in_len, void *arg)
//    {
//      static const char*const h2_proto_string = "\x02h2";
//      std::size_t h2_proto_string_len = ::strlen(h2_proto_string);
//      const unsigned char* client_proto = in;
//      const unsigned char* client_proto_end = in + in_len;
//      for ( ; client_proto + h2_proto_string_len <= client_proto_end; client_proto += *client_proto + 1)
//      {
//        std::size_t client_proto_len = (*client_proto + 1);
//        if (::memcmp(h2_proto_string, client_proto, h2_proto_string_len <  client_proto_len ? h2_proto_string_len : client_proto_len) == 0)
//        {
//          *out = client_proto;
//          *out_len = (unsigned char)client_proto_len;
//          return SSL_TLSEXT_ERR_OK;
//        }
//      }
//      return SSL_TLSEXT_ERR_NOACK;
//    }

//    //----------------------------------------------------------------//
//    void client::v2_connection::on_informational_headers(std::uint32_t stream_id, const std::function<void(v2_response_head&& headers)>& fn)
//    {
//      auto it = this->streams_.find(stream_id);
//      if (it == this->streams_.end())
//      {
//        // TODO: Handle error
//      }
//      else
//      {
//        ((stream*)it->second.get())->on_informational_headers(fn);
//      }
//    }
//    //----------------------------------------------------------------//
//
//    //----------------------------------------------------------------//
//    void client::v2_connection::on_response(std::uint32_t stream_id, const std::function<void(http::client::v2_response && resp)>& fn)
//    {
//      auto self = shared_from_this();
//      auto it = this->streams_.find(stream_id);
//      if (it == this->streams_.end())
//      {
//        // TODO: Handle error
//      }
//      else
//      {
//        ((stream*)it->second.get())->on_response_headers([self, fn, stream_id](v2_response_head&& headers)
//        {
//          fn(http::client::response(std::move(headers), self, stream_id));
//        });
//      }
//    }
//    //----------------------------------------------------------------//
//
//    //----------------------------------------------------------------//
//    void client::v2_connection::on_trailers(std::uint32_t stream_id, const std::function<void(v2_header_block&& headers)>& fn)
//    {
//      auto it = this->streams_.find(stream_id);
//      if (it == this->streams_.end())
//      {
//        // TODO: Handle error
//      }
//      else
//      {
//        ((stream*)it->second.get())->on_trailers(fn);
//      }
//    }
//    //----------------------------------------------------------------//
//
//    //----------------------------------------------------------------//
//    void client::v2_connection::on_push_promise(std::uint32_t stream_id, const std::function<void(http::client::v2_request && req)>& fn)
//    {
//      auto self = this->shared_from_this();
//      auto it = this->streams_.find(stream_id);
//      if (it == this->streams_.end())
//      {
//      // TODO: Handle error
//      }
//      else
//      {
//        it->second->on_push_promise([self, fn, stream_id](v2_request_head&& headers, std::uint32_t promised_stream_id)
//        {
//          fn(http::client::request(std::move(headers), self, promised_stream_id));
//        });
//      }
//    }

    //----------------------------------------------------------------//
    client::request::request(request_head&& head, const std::shared_ptr<http::connection<request_head, response_head>>& conn, std::uint32_t stream_id, const std::string& server_authority)
      : outgoing_message(conn, stream_id), head_(std::move(head)), server_authority_(server_authority)
    {
      conn->on_push_promise(stream_id, [conn](v2_header_block&& req, std::uint32_t promised_stream_id)
      {
        conn->send_reset_stream(promised_stream_id, errc::refused_stream);
      });
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    client::request::request(request&& source)
     : outgoing_message(std::move(source)), head_(std::move(source.head_)), server_authority_(std::move(source.server_authority_))
    {
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    client::request::~request()
    {
      if (this->connection_)
        this->end();
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
      if (this->head_.method() == "GET" || this->head_.method() == "HEAD")
        end_stream = true;
      if (this->head().authority().empty())
        this->head().authority(this->server_authority_);
      return outgoing_message::send_headers(end_stream);
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void client::request::on_push_promise(const std::function<void(http::client::request && request)>& cb)
    {
      auto c = this->connection_;
      std::uint32_t stream_id = this->stream_id_;
      this->connection_->on_push_promise(this->stream_id_, [cb, c, stream_id](request_head&& headers, std::uint32_t promised_stream_id)
      {
        std::string authority = headers.authority();
        cb ? cb(http::client::request(std::move(headers), c, promised_stream_id, authority)) : void();
      });
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void client::request::on_informational_headers(const std::function<void(response_head&& resp_head)>& cb)
    {
      this->connection_->on_informational_headers(this->stream_id_, cb) ;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void client::request::on_response(const std::function<void(http::client::response && resp)>& cb)
    {
      auto c = this->connection_;
      std::uint32_t stream_id = this->stream_id_;
      this->connection_->on_headers(this->stream_id_, [cb, c, stream_id](response_head&& headers)
      {
        cb ? cb(http::client::response(std::move(headers), c, stream_id)) : void();
      });
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    client::response::response(response_head&& head, const std::shared_ptr<http::connection<request_head, response_head>>& conn, std::uint32_t stream_id)
      : incoming_message(conn, stream_id)
    {
      this->head_ = std::move(head);
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    client::response::response(response&& source)
      : incoming_message(std::move(source)), head_(std::move(source.head_))
    {
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
      : impl_(std::make_shared<client_impl>(ioservice))
    {
      this->impl_->connect(host, port);
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    client::client(asio::io_service& ioservice, const std::string& host, const ssl_options& options, unsigned short port)
        : impl_(std::make_shared<client_impl>(ioservice))
    {
      this->impl_->connect(host, options, port);
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    client::client(client&& source)
    {
      this->impl_ = source.impl_;
      source.impl_ = nullptr;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    client::~client()
    {
      if (this->impl_)
        this->impl_->close(errc::cancel);
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void client::close(errc ec)
    {
      this->impl_->close(ec);
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    http::client::request client::make_request()
    {
      return this->impl_->make_request();
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void client::on_connect(const std::function<void()>& fn)
    {
      this->impl_->on_connect(fn);
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void client::on_close(const std::function<void(errc ec)>& fn)
    {
      this->impl_->on_close(fn);
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void client::set_default_user_agent(const std::string user_agent)
    {
      this->impl_->set_default_user_agent(user_agent);
    }
    //----------------------------------------------------------------//
  }
}
