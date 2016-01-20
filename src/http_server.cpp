
#include <memory>
#include <array>
#include <iostream>

#include "asio.hpp"
#include "tcp.hpp"
#include "http_server.hpp"
#include "http_v2_connection.hpp"
#include "http_v1_connection.hpp"

namespace manifold
{
  namespace http
  {
    int alpn_select_proto_cb(SSL *ssl, const unsigned char **out,
      unsigned char *out_len, const unsigned char *in,
      unsigned int in_len, void *arg)
    {
      //static const char*const h2_proto_string = "\x02h2\x08http/1.1";
      std::size_t h2_proto_string_len = ::strlen(MANIFOLD_HTTP_ALPN_SUPPORTED_PROTOCOLS);

      int ret = SSL_select_next_proto((unsigned char **)out, out_len, (unsigned char*)MANIFOLD_HTTP_ALPN_SUPPORTED_PROTOCOLS, h2_proto_string_len, in, in_len) == OPENSSL_NPN_NEGOTIATED ? SSL_TLSEXT_ERR_OK : SSL_TLSEXT_ERR_ALERT_FATAL;
      auto select_proto = *out;
      int e = SSL_get_error(ssl, ret);
      return  ret;
//      const unsigned char* client_proto = in;
//      const unsigned char* client_proto_end = in + in_len;
//      for ( ; client_proto + h2_proto_string_len <= client_proto_end; client_proto += *client_proto + 1)
//      {
//        std::size_t client_proto_len = (*client_proto + 1);
//        if (::memcmp(h2_proto_string, client_proto, h2_proto_string_len <  client_proto_len ? h2_proto_string_len : client_proto_len) == 0)
//        {
//          *out = client_proto + 1;
//          *out_len = (unsigned char)(client_proto_len - 1);
//          return SSL_TLSEXT_ERR_OK;
//        }
//      }
      return SSL_TLSEXT_ERR_NOACK;
    }

    //----------------------------------------------------------------//
    server::request::request(request_head&& head, const std::shared_ptr<http::connection<response_head, request_head>>& conn, std::int32_t stream_id)
      : incoming_message<response_head, request_head>(conn, stream_id)
    {
      this->head_ = std::move(head);
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    server::request::request(server::request&& source)
      : incoming_message(std::move(source)), head_(std::move(source.head_))
    {
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    server::request::~request()
    {
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    const request_head& server::request::head() const
    {
      return this->head_;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    server::response::response(response_head&& head, const std::shared_ptr<http::connection<response_head, request_head>>& conn, std::int32_t stream_id, const std::string& request_method)
      : outgoing_message<response_head, request_head>(conn, stream_id), request_method_(request_method)
    {
      this->head_ = std::move(head);
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    server::response::response(server::response&& source)
      : outgoing_message(std::move(source)), head_(std::move(source.head_)), request_method_(std::move(source.request_method_))
    {
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    server::response::~response()
    {
      if (this->connection_)
        this->end();
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    response_head& server::response::head()
    {
      return this->head_;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    bool server::response::send_headers(bool end_stream)
    {
      if (this->head().header("date").empty())
        this->head().header("date", server::date_string());
      if (this->request_method_ == "HEAD")
        end_stream = true;
      return outgoing_message::send_headers(end_stream);
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    server::push_promise server::response::send_push_promise(request_head&& push_promise_headers)
    {
      push_promise ret;
      std::string method = push_promise_headers.method();
      std::uint32_t promised_stream_id = this->connection_->send_push_promise(this->stream_id_, push_promise_headers);
      if (promised_stream_id)
      {
        ret = push_promise(server::request(std::move(push_promise_headers), this->connection_, promised_stream_id), server::response(response_head(200, {{"server", this->head().header("server")}}), this->connection_, promised_stream_id, method));
      }
      return ret;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    server::push_promise server::response::send_push_promise(const request_head& push_promise_headers)
    {
      return this->send_push_promise(request_head(push_promise_headers));
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    server::push_promise::push_promise()
      : fulfilled_(false)
    {
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    server::push_promise::push_promise(request&& req, response&& res)
      : req_(new request(std::move(req))), res_(new response(std::move(res))), fulfilled_(false)
    {

    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void server::push_promise::fulfill(const std::function<void(server::request&& req, server::response&& res)>& handler)
    {
      if (!this->fulfilled_ && this->req_ && this->res_)
      {
        this->fulfilled_ = true;
        handler(std::move(*req_), std::move(*res_));
      }
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    server::server(asio::io_service& ioservice, unsigned short port, const std::string& host)
      : io_service_(ioservice),
        acceptor_(io_service_)
    {
      this->port_ = port;
      this->host_ = host;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    server::server(asio::io_service& ioservice, ssl_options options, unsigned short port, const std::string& host)
      : io_service_(ioservice),
      acceptor_(io_service_),
      ssl_context_(new asio::ssl::context(options.method))
    {
//      this->ssl_context_->set_options(
//        asio::ssl::context::default_workarounds
//          | asio::ssl::context::no_sslv2
//          | asio::ssl::context::single_dh_use);
      if (true) //options.pfx.size())
      {
        //this->ssl_context_->use_certificate_chain_file("/Users/jonathonl/Developer/certs2/server-cert.pem");
        //this->ssl_context_->use_private_key_file("/Users/jonathonl/Developer/certs2/server-key.pem", asio::ssl::context::pem);
        //this->ssl_context_->use_tmp_dh_file("/Users/jonathonl/Developer/certs/dh512.pem");

        char cwd[FILENAME_MAX];
        getcwd(cwd, FILENAME_MAX);

        //this->ssl_context_->use_certificate_chain_file("tests/certs/server.crt");
        //this->ssl_context_->use_private_key_file("tests/certs/server.key", asio::ssl::context::pem);
        //this->ssl_context_->use_tmp_dh_file("tests/certs/dh2048.pem");

        this->ssl_context_->use_certificate_chain(asio::buffer(options.chain.data(), options.chain.size()));
        this->ssl_context_->use_private_key(asio::buffer(options.key.data(), options.key.size()), asio::ssl::context::pem);
        this->ssl_context_->use_tmp_dh(asio::buffer(options.dhparam.data(), options.dhparam.size()));
      }
      else
      {
        std::error_code ec;
        if (options.cert.size())
          this->ssl_context_->use_certificate_chain(asio::const_buffer(options.cert.data(), options.cert.size()));
        if (options.key.size())
          this->ssl_context_->use_private_key(asio::const_buffer(options.key.data(), options.key.size()), asio::ssl::context::file_format::pem);
        if (options.ca.size())
          this->ssl_context_->use_tmp_dh_file("/Users/jonathonl/Developer/certs/dh512.pem");
      }


      auto ssl_opts = (SSL_OP_ALL & ~SSL_OP_DONT_INSERT_EMPTY_FRAGMENTS) |
        SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_COMPRESSION |
        SSL_OP_NO_SESSION_RESUMPTION_ON_RENEGOTIATION |
        SSL_OP_SINGLE_ECDH_USE | SSL_OP_NO_TICKET |
        SSL_OP_CIPHER_SERVER_PREFERENCE;

      SSL_CTX_set_options(ssl_context_->native_handle(), SSL_CTX_get_options(ssl_context_->native_handle()) | SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3);
      //SSL_CTX_set_options(ssl_context_->impl(), ssl_opts);
      //SSL_CTX_set_mode(ssl_context_->impl(), SSL_MODE_AUTO_RETRY);
      //SSL_CTX_set_mode(ssl_context_->impl(), SSL_MODE_RELEASE_BUFFERS);

      static const char *const DEFAULT_CIPHER_LIST =
        //"HIGH:!AES256-SHA:!AES128-GCM-SHA256:!AES128-SHA:!DES-CBC3-SHA";
        //"DHE:EDH:kDHE:kEDH:DH:kEECDH:kECDHE:ECDHE:EECDH:ECDH";
        "ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-"
        "AES256-GCM-SHA384:ECDHE-ECDSA-AES256-GCM-SHA384:DHE-RSA-AES128-GCM-SHA256:"
        "DHE-DSS-AES128-GCM-SHA256:kEDH+AESGCM:ECDHE-RSA-AES128-SHA256:ECDHE-ECDSA-"
        "AES128-SHA256:ECDHE-RSA-AES128-SHA:ECDHE-ECDSA-AES128-SHA:ECDHE-RSA-"
        "AES256-SHA384:ECDHE-ECDSA-AES256-SHA384:ECDHE-RSA-AES256-SHA:ECDHE-ECDSA-"
        "AES256-SHA:DHE-RSA-AES128-SHA256:DHE-RSA-AES128-SHA:DHE-DSS-AES128-SHA256:"
        "DHE-RSA-AES256-SHA256:DHE-DSS-AES256-SHA:DHE-RSA-AES256-SHA:!aNULL:!eNULL:"
        "!EXPORT:!DES:!RC4:!3DES:!MD5:!PSK";



      //SSL_CTX_set_cipher_list(ssl_context_->impl(), DEFAULT_CIPHER_LIST);


      SSL_CTX_set_alpn_select_cb(this->ssl_context_->impl(), alpn_select_proto_cb, nullptr);
      this->port_ = port;
      this->host_ = host;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    server::~server()
    {
      this->close();
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void server::listen(const std::function<void(server::request&& req, server::response&& res)>& handler)
    {
      this->request_handler_ = handler;
      // Open the acceptor with the option to reuse the address (i.e. SO_REUSEADDR).
      //asio::ip::tcp::resolver resolver(io_service_);
      //asio::ip::tcp::endpoint endpoint = *(resolver.resolve({host, std::to_string(port)}));
      auto ep = asio::ip::tcp::endpoint(asio::ip::address::from_string(this->host_), this->port_);
      acceptor_.open(ep.protocol());
      acceptor_.set_option(asio::ip::tcp::acceptor::reuse_address(true));
      acceptor_.bind(ep);
      acceptor_.listen();

      if (this->ssl_context_)
        this->accept(*this->ssl_context_);
      else
        this->accept();
    };
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void server::close()
    {
      this->acceptor_.close();
      std::set<std::shared_ptr<http::connection<response_head, request_head>>> tmp;
      tmp.swap(this->connections_);
      for (auto it = tmp.begin(); it != tmp.end(); ++it)
        (*it)->close(errc::cancel);
      tmp.clear();
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void server::accept()
    {
      if (acceptor_.is_open())
      {
        auto sock = std::make_shared<non_tls_socket>(this->io_service_);
        acceptor_.async_accept((asio::ip::tcp::socket&)*sock, [this, sock](std::error_code ec)
        {
          if (ec)
          {
            std::cout << "accept error: " << ec.message() << std::endl;
          }
          else
          {
            auto it = this->connections_.emplace(std::make_shared<v1_connection<response_head, request_head>>(std::move(*sock)));
            if (it.second)
            {
              this->manage_connection(*it.first);
              (*it.first)->run();
            }
          }

          this->accept();
        });
      }
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void server::accept(asio::ssl::context& ctx)
    {
      if (acceptor_.is_open())
      {
        auto sock = std::make_shared<tls_socket>(this->io_service_, ctx);
        acceptor_.async_accept(((asio::ssl::stream<asio::ip::tcp::socket>&)*sock).lowest_layer(), [this, sock, &ctx](std::error_code ec)
        {
          if (ec)
          {
            std::cout << "accept error: " << ec.message() << std::endl;
          }
          else
          {
            ((asio::ssl::stream<asio::ip::tcp::socket>&)*sock).async_handshake(asio::ssl::stream_base::server, [this, sock] (const std::error_code& ec)
            {
              std::cout << "Cipher: " << SSL_CIPHER_get_name(SSL_get_current_cipher(((asio::ssl::stream<asio::ip::tcp::socket>&)*sock).native_handle())) << std::endl;
              const unsigned char* selected_alpn = nullptr;
              unsigned int selected_alpn_sz = 0;
              SSL_get0_alpn_selected(((asio::ssl::stream<asio::ip::tcp::socket>&)*sock).native_handle(), &selected_alpn, &selected_alpn_sz);
              std::cout << "Server ALPN: " << std::string((char*)selected_alpn, selected_alpn_sz) << std::endl;
              if (ec)
              {
                std::cout << ec.message() << ":" __FILE__ << "/" << __LINE__ << std::endl;
              }
  #ifndef MANIFOLD_DISABLE_HTTP2
              else if (std::string((char*)selected_alpn, selected_alpn_sz) == "h2")
              {
                auto* preface_buf = new std::array<char,v2_connection::preface.size()>();
                sock->recv(preface_buf->data(), v2_connection::preface.size(), [this, sock, preface_buf](const std::error_code& ec, std::size_t bytes_read)
                {
                  if (ec)
                  {
                    std::cout << ec.message() << ":" __FILE__ << "/" << __LINE__ << std::endl;
                    std::string err = ec.message();
                    if (ec.category() == asio::error::get_ssl_category())
                    {
                      err = std::string(" (");
                      //ERR_PACK /* crypto/err/err.h */
                      char buf[128];
                      ::ERR_error_string_n(ec.value(), buf, sizeof(buf));
                      err += buf;
                    }
                  }
                  else
                  {
                    const char* t = preface_buf->data();
                    if (*preface_buf != v2_connection::preface)
                    {
                      std::cout << "Invalid Connection Preface" << ":" __FILE__ << "/" << __LINE__ << std::endl;
                    }
                    else
                    {
                      auto it = this->connections_.emplace(std::make_shared<server::v2_connection>(std::move(*sock)));
                      if (it.second)
                      {
                        this->manage_connection(*it.first);
                        (*it.first)->run();
                      }
                    }
                  }
                  delete preface_buf;
                });
              }
  #endif //MANIFOLD_DISABLE_HTTP2
              else
              {
                auto it = this->connections_.emplace(std::make_shared<v1_connection<response_head, request_head>>(std::move(*sock)));
                if (it.second)
                {
                  this->manage_connection(*it.first);
                  (*it.first)->run();
                }
              }
            });
          }

          this->accept(ctx);
        });
      }
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void server::manage_connection(const std::shared_ptr<http::connection<response_head, request_head>>& conn)
    {
      conn->on_new_stream([this, conn](std::int32_t stream_id)
      {
        conn->on_headers(stream_id, [conn, stream_id, this](request_head&& headers)
        {
          std::string method = headers.method();
          this->request_handler_ ? this->request_handler_(server::request(std::move(headers), conn, stream_id), server::response(response_head(), conn, stream_id, method)) : void();
        });

        conn->on_push_promise(stream_id, [stream_id, conn](response_head&& head, std::uint32_t promised_stream_id)
        {
          conn->send_goaway(errc::protocol_error, "Clients Cannot Push!");
        });
      });

      conn->on_close([conn, this](errc ec)
      {
        this->connections_.erase(conn);
      });
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void server::set_default_server_header(const std::string& value)
    {
      this->default_server_header_ = value;
    }
    //----------------------------------------------------------------//
  }
}
