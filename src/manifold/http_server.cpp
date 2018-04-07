
#include <memory>
#include <array>
#include <iostream>

#include <asio/spawn.hpp>

#include "manifold/http_server.hpp"
#include "manifold/http_connection.hpp"

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
    server::request::request(request_head&& head, const std::shared_ptr<connection::stream>& stream_ptr)
      : incoming_message(stream_ptr)
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
    server::response::response(response_head&& head, const std::shared_ptr<connection::stream>& stream_ptr, const std::string& request_method, const std::string& request_authority)
      : outgoing_message(stream_ptr), request_method_(request_method), request_authority_(request_authority)
    {
      this->head_ = std::move(head);
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    server::response::response(server::response&& source)
      : outgoing_message(std::move(source)), head_(std::move(source.head_)), request_method_(std::move(source.request_method_)), request_authority_(std::move(source.request_authority_))
    {
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    server::response::~response()
    {
      if (this->stream_)
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
    future<bool> server::response::send_headers(bool end_stream)
    {
      if (this->head().header("date").empty())
        this->head().header("date", server::date_string());
      if (this->request_method_ == "HEAD")
        end_stream = true;
      bool ret = co_await outgoing_message::send_headers(end_stream);
      co_return ret;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    server::push_promise server::response::send_push_promise(request_head&& push_promise_headers)
    {
      push_promise ret;
      // TODO !!!
//      if (this->connection_)
//      {
//        std::string method = push_promise_headers.method();
//        std::uint32_t promised_stream_id = this->connection_->send_push_promise(this->stream_id_, push_promise_headers);
//        if (promised_stream_id)
//        {
//          if (push_promise_headers.authority().empty())
//            push_promise_headers.authority(this->request_authority_);
//          ret = push_promise(server::request(std::move(push_promise_headers), this->connection_, promised_stream_id), server::response(response_head(200, {{"server", this->head().header("server")}}), this->connection_, promised_stream_id, method, this->request_authority_));
//        }
//      }

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
        acceptor_(io_service_),
        ssl_context_(nullptr)
    {
      this->port_ = port;
      this->host_ = host;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    server::server(asio::io_service& ioservice, asio::ssl::context& ctx, unsigned short port, const std::string& host)
      : io_service_(ioservice),
      acceptor_(io_service_),
      ssl_context_(&ctx)
    {
//      auto ssl_opts = (SSL_OP_ALL & ~SSL_OP_DONT_INSERT_EMPTY_FRAGMENTS) |
//        SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_COMPRESSION |
//        SSL_OP_NO_SESSION_RESUMPTION_ON_RENEGOTIATION |
//        SSL_OP_SINGLE_ECDH_USE | SSL_OP_NO_TICKET |
//        SSL_OP_CIPHER_SERVER_PREFERENCE;

      SSL_CTX_set_options(ssl_context_->native_handle(), SSL_CTX_get_options(ssl_context_->native_handle()) | SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3);

      ec_key_st* ecdh = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
      if (ecdh)
      {
        SSL_CTX_set_tmp_ecdh(ssl_context_->native_handle(), ecdh);
        EC_KEY_free(ecdh);
      }

      static const char *const DEFAULT_CIPHER_LIST = "ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-AES256-GCM-SHA384:DHE-RSA-AES128-GCM-SHA256:DHE-DSS-AES128-GCM-SHA256:kEDH+AESGCM:ECDHE-RSA-AES128-SHA256:ECDHE-ECDSA-AES128-SHA256:ECDHE-RSA-AES128-SHA:ECDHE-ECDSA-AES128-SHA:ECDHE-RSA-AES256-SHA384:ECDHE-ECDSA-AES256-SHA384:ECDHE-RSA-AES256-SHA:ECDHE-ECDSA-AES256-SHA:DHE-RSA-AES128-SHA256:DHE-RSA-AES128-SHA:DHE-DSS-AES128-SHA256:DHE-RSA-AES256-SHA256:DHE-DSS-AES256-SHA:DHE-RSA-AES256-SHA:!aNULL:!eNULL:!EXPORT:!DES:!RC4:!3DES:!MD5:!PSK";

      std::cout << "SSL_CTX_set_cipher_list: " << ::SSL_CTX_set_cipher_list(ssl_context_->native_handle(), DEFAULT_CIPHER_LIST) << std::endl;

      ::SSL_CTX_set_alpn_select_cb(this->ssl_context_->native_handle(), alpn_select_proto_cb, nullptr);
      this->port_ = port;
      this->host_ = host;
    }
    //----------------------------------------------------------------//

    void server::reset_timeout(std::chrono::system_clock::duration value)
    {
      this->timeout_ = value;
    }

    //----------------------------------------------------------------//
    void server::close()
    {
      if (!this->closed_)
      {
        this->closed_ = true;

        this->acceptor_.close();
        std::set<std::unique_ptr<http::connection>> tmp;
        tmp.swap(this->connections_);
        for (auto it = tmp.begin(); it != tmp.end(); ++it)
          (*it)->close(v2_errc::cancel);
        tmp.clear();
      }
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    future<void> server::accept()
    {
      while (acceptor_.is_open() && !this->closed_)
      {
        std::error_code ec;
        non_tls_socket sock(io_service_);
        co_await async_accept(acceptor_, (asio::ip::tcp::socket&)sock, ec);
        if (ec)
        {
          std::cout << "accept error: " << ec.message() << std::endl;
        }
        else
        {
          auto res = this->connections_.emplace(std::make_unique<connection>(std::move(sock), http::version::http1_1, nullptr));
          if (res.second)
          {
            //res.first->run(this->timeout_); //TODO: allow to configure.
          }
        }
      }
      co_return;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    future<void> server::accept(asio::ssl::context& ctx)
    {
      while (acceptor_.is_open() && !this->closed_)
      {
        std::error_code ec;
        tls_socket sock(this->io_service_, ctx);
        co_await async_accept(acceptor_, ((asio::ssl::stream<asio::ip::tcp::socket>&)sock).lowest_layer(), ec);
        if (ec)
        {
          std::cout << "accept error: " << ec.message() << std::endl;
        }
        else
        {
          co_await async_handshake(((asio::ssl::stream<asio::ip::tcp::socket>&)sock), asio::ssl::stream_base::server, ec);

          std::cout << "Cipher: " << SSL_CIPHER_get_name(SSL_get_current_cipher(((asio::ssl::stream<asio::ip::tcp::socket>&)sock).native_handle())) << std::endl;
          const unsigned char* selected_alpn = nullptr;
          unsigned int selected_alpn_sz = 0;
          SSL_get0_alpn_selected(((asio::ssl::stream<asio::ip::tcp::socket>&)sock).native_handle(), &selected_alpn, &selected_alpn_sz);
          std::cout << "Server ALPN: " << std::string((char*)selected_alpn, selected_alpn_sz) << std::endl;
          if (ec)
          {
            std::cout << ec.message() << ":" __FILE__ << "/" << __LINE__ << std::endl;
          }
#ifndef MANIFOLD_DISABLE_HTTP2
          else if (std::string((char*)selected_alpn, selected_alpn_sz) == "h2")
          {
            auto* preface_buf = new std::array<char, connection::http2_preface.size()>();
            std::size_t bytes_read = co_await sock.recv(preface_buf->data(), preface_buf->size(), ec);

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
              if (*preface_buf != connection::http2_preface)
              {
                std::cout << "Invalid Connection Preface" << ":" __FILE__ << "/" << __LINE__ << std::endl;
              }
              else
              {
                auto res = this->connections_.emplace(std::make_unique<connection>(std::move(sock), http::version::http2, std::bind(&server::new_stream_handler, this, std::placeholders::_1)));

                if (res.second)
                {

                  //c->run(this->timeout_, {});
                }
              }
            }
            delete preface_buf;
          }
#endif //MANIFOLD_DISABLE_HTTP2
          else
          {
            auto res = this->connections_.emplace(std::make_unique<connection>(std::move(sock), http::version::http1_1, nullptr));
            if (res.second)
            {
              //res.second->run(this->timeout_); // TODO: Allow to configure
            }
          }
        }
      }
      co_return;
    }
    //----------------------------------------------------------------//

    future<void> server::new_stream_handler(std::shared_ptr<connection::stream> stream_ptr)
    {
      http::header_block headers = co_await stream_ptr->recv_headers();
      //std::string method = headers.method();
      //std::string authority = headers.authority();
      //this->request_handler_ ? this->request_handler_(server::request(std::move(headers), conn, stream_id), server::response(response_head(), conn, stream_id, method, authority)) : void();
      if (this->request_handler_)
        this->request_handler_(server::request(std::move(headers), stream_ptr), server::response(response_head(), stream_ptr, "", ""));
    }

// TODO: default push_promise and cleaning up stale connections.
//   void server_impl::manage_connection(http::connection& conn)
//   {
//      conn->on_new_stream([this, conn](std::int32_t stream_id)
//      {
//        conn->on_headers(stream_id, [conn, stream_id, this](request_head&& headers)
//        {
//          std::string method = headers.method();
//          std::string authority = headers.authority();
//          this->request_handler_ ? this->request_handler_(server::request(std::move(headers), conn, stream_id), server::response(response_head(), conn, stream_id, method, authority)) : void();
//        });
//
//        conn->on_push_promise(stream_id, [stream_id, conn](response_head&& head, std::uint32_t promised_stream_id)
//        {
//          conn->send_goaway(v2_errc::protocol_error, "Clients Cannot Push!");
//        });
//      });
//
//      conn->on_close([conn, this](const std::error_code& ec)
//      {
//        this->connections_.erase(conn);
//      });
//  }


    //----------------------------------------------------------------//
    void server::set_default_server_header(const std::string& value)
    {
      this->default_server_header_ = value;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    server::~server()
    {
      this->close();
    }
    //----------------------------------------------------------------//
  }
}
