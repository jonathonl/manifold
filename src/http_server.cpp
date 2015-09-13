
#include <memory>
#include <array>
#include <iostream>

#include "asio.hpp"
#include "tcp.hpp"
#include "http_server.hpp"
#include "http_connection.hpp"

namespace manifold
{
  namespace http
  {
    //----------------------------------------------------------------//
    server::request::request(request_head&& head, const std::shared_ptr<http::connection>& conn, std::int32_t stream_id)
      : incoming_message(conn, stream_id)
    {
      this->head_ = std::move(head);
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
    server::response::response(response_head&& head, const std::shared_ptr<http::connection>& conn, std::int32_t stream_id)
      : outgoing_message(conn, stream_id)
    {
      this->head_ = std::move(head);
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    server::response::~response()
    {

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
      return outgoing_message::send_headers(end_stream);
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    server::server(asio::io_service& ioService, unsigned short port, const std::string& host)
      : io_service_(ioService),
        acceptor_(io_service_)
    {
      this->port_ = port;
      this->host_ = host;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    server::server(asio::io_service& ioService, ssl_options options, unsigned short port, const std::string& host)
      : io_service_(ioService),
      acceptor_(io_service_),
      ssl_context_(new asio::ssl::context(options.method))
    {
      this->port_ = port;
      this->host_ = host;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    server::~server()
    {
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
    void server::accept()
    {
      auto sock = std::make_shared<non_tls_socket>(this->io_service_);
      acceptor_.async_accept((asio::ip::tcp::socket&)*sock, [this, sock](std::error_code ec)
      {

        if (!acceptor_.is_open())
        {
          std::cout << "acceptor not open" << ":" __FILE__ << "/" << __LINE__ << std::endl;
          return;
        }

        if (!ec)
        {
          auto* preface_buf = new std::array<char,connection::preface.size()>();
          sock->recv(preface_buf->data(), connection::preface.size(), [this, sock, preface_buf](const std::error_code& ec, std::size_t bytes_read)
          {
            if (ec)
            {
              std::cout << ec.message() << ":" __FILE__ << "/" << __LINE__ << std::endl;
            }
            else
            {
              if (*preface_buf != connection::preface)
              {
                std::cout << "Invalid Connection Preface" << ":" __FILE__ << "/" << __LINE__ << std::endl;
              }
              else
              {
                auto it = this->connections_.emplace(std::make_shared<connection>(std::move(*sock)));
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
        else
        {
          std::cout << ec.message() << ":" __FILE__ << "/" << __LINE__ << std::endl;
        }

        if (!this->io_service_.stopped())
          this->accept();
      });
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void server::accept(asio::ssl::context& ctx)
    {
      auto sock = std::make_shared<tls_socket>(this->io_service_, ctx);
      acceptor_.async_accept(((asio::ssl::stream<asio::ip::tcp::socket>&)*sock).next_layer(), [this, sock, &ctx](std::error_code ec)
      {

        if (!acceptor_.is_open())
        {
          std::cout << "acceptor not open" << ":" __FILE__ << "/" << __LINE__ << std::endl;
          return;
        }

        if (ec)
        {
        }
        else
        {
          ((asio::ssl::stream<asio::ip::tcp::socket>&)*sock).async_handshake(asio::ssl::stream_base::server, [this, sock] (const std::error_code& ec)
          {
            if (ec)
            {
              std::cout << ec.message() << ":" __FILE__ << "/" << __LINE__ << std::endl;
            }
            else
            {
              auto* preface_buf = new std::array<char,connection::preface.size()>();
              sock->recv(preface_buf->data(), connection::preface.size(), [this, sock, preface_buf](const std::error_code& ec, std::size_t bytes_read)
              {
                if (ec)
                {
                  std::cout << ec.message() << ":" __FILE__ << "/" << __LINE__ << std::endl;
                }
                else
                {
                  if (*preface_buf != connection::preface)
                  {
                    std::cout << "Invalid Connection Preface" << ":" __FILE__ << "/" << __LINE__ << std::endl;
                  }
                  else
                  {
                    auto it = this->connections_.emplace(std::make_shared<connection>(std::move(*sock)));
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
          });
        }

        if (!this->io_service_.stopped())
          this->accept(ctx);
      });
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void server::manage_connection(const std::shared_ptr<http::connection>& conn)
    {
      conn->on_new_stream([this, conn](std::int32_t stream_id)
      {
        conn->on_headers(stream_id, [conn, stream_id, this](http::header_block&& headers)
        {
          this->request_handler_ ? this->request_handler_(server::request(std::move(headers), conn, stream_id), server::response(http::response_head(200, {{"server", this->default_server_header_}}), conn, stream_id)) : void();
        });
      });

      conn->on_close([conn, this](std::uint32_t ec)
      {
        this->connections_.erase(conn);
        this->io_service_.post([conn]()
        {
          conn->close();
        });
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
