
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
      : incoming_message(this->head_, conn, stream_id)
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
      : outgoing_message(this->head_, conn, stream_id)
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
    server::server(asio::io_service& ioService)
      : io_service_(ioService),
        acceptor_(io_service_),
        socket_(io_service_)
    {

    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    server::~server()
    {
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void server::listen(unsigned short port, const std::string& host)
    {
      // Open the acceptor with the option to reuse the address (i.e. SO_REUSEADDR).
      asio::ip::tcp::resolver resolver(io_service_);
      asio::ip::tcp::endpoint endpoint = *(resolver.resolve({host, std::to_string(port)}));
      acceptor_.open(endpoint.protocol());
      acceptor_.set_option(asio::ip::tcp::acceptor::reuse_address(true));
      acceptor_.bind(endpoint);
      acceptor_.listen();

      this->accept();
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void server::accept()
    {
      acceptor_.async_accept(this->socket_,
        [this](std::error_code ec)
        {
          std::cout << "ACCEPT:" << this->socket_.native_handle() << std::endl;
          // Check whether the server was stopped by a signal before this
          // completion handler had a chance to run.
          if (!acceptor_.is_open())
          {
            std::cout << "acceptor not open" << ":" __FILE__ << "/" << __LINE__ << std::endl;
            return;
          }

          if (!ec)
          {
            auto it = this->connections_.emplace(std::make_shared<connection>(std::move(this->socket_)));
            this->socket_ = asio::ip::tcp::socket(this->io_service_);
            if (it.second)
              (*it.first)->run();
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
    void server::manage_connection(const std::shared_ptr<http::connection>& conn)
    {
      conn->on_new_stream([this, conn](std::int32_t stream_id, std::list<std::pair<std::string,std::string>>&& headers, std::int32_t stream_dependency_id)
      {
        std::function<void(server::request&& req, server::response&& res)> fn = this->stream_handlers_.begin()->second;

        if (fn) fn(server::request(http::request_head(), conn, stream_id), server::response(http::response_head(), conn, stream_id));
      });

      conn->on_close([conn, this]()
      {
        this->connections_.erase(conn);
      });
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void server::register_handler(const std::regex& expression, const std::function<void(server::request&& req, server::response&& res)>& handler)
    {
      this->stream_handlers_.push_back(std::make_pair(expression, handler));
    }
    //----------------------------------------------------------------//

  }
}
