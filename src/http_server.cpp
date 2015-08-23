
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
      std::shared_ptr<connection> conn;
      conn = std::make_shared<non_tls_connection>(this->io_service_);
      acceptor_.async_accept(conn->socket(), [this, conn](std::error_code ec)
      {

        if (!acceptor_.is_open())
        {
          std::cout << "acceptor not open" << ":" __FILE__ << "/" << __LINE__ << std::endl;
          return;
        }

        if (!ec)
        {
          auto it = this->connections_.emplace(conn);
          if (it.second)
          {
            this->manage_connection(*it.first);
            (*it.first)->run();
          }
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
      std::shared_ptr<tls_connection> conn;
      conn = std::make_shared<tls_connection>(this->io_service_, ctx);
      acceptor_.async_accept(conn->socket(), [this, conn, &ctx](std::error_code ec)
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
          conn->ssl_stream().async_handshake(asio::ssl::stream_base::server, [this, conn] (const std::error_code& ec)
          {
            if (ec)
            {
              std::cout << ec.message() << ":" __FILE__ << "/" << __LINE__ << std::endl;
            }
            else
            {
              auto it = this->connections_.emplace(conn);
              if (it.second)
              {
                this->manage_connection(*it.first);
                (*it.first)->run();
              }
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
      conn->on_new_stream([this, conn](std::int32_t stream_id, header_block&& headers)
      {
        this->request_handler_ ? this->request_handler_(server::request(std::move(headers), conn, stream_id), server::response(http::response_head(), conn, stream_id)) : void();
      });

      conn->on_close([conn, this](std::uint32_t ec)
      {
        this->connections_.erase(conn);
      });
    }
    //----------------------------------------------------------------//
  }
}
