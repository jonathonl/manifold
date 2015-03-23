
#include "tcp.hpp"
#include "http_server.hpp"

namespace IPSuite
{
  namespace HTTP
  {
    //----------------------------------------------------------------//
    Server::Request::Request(RequestHead&& head, Socket& sock)
      : IncomingMessage(this->head_, sock)
    {
      this->head_ = std::move(head);
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    Server::Request::~Request()
    {
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    const RequestHead& Server::Request::head() const
    {
      return this->head_;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    Server::Response::Response(ResponseHead&& head, Socket& sock)
      : OutgoingMessage(this->head_, sock)
    {
      this->head_ = std::move(head);
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    Server::Response::~Response()
    {

    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    ResponseHead& Server::Response::head()
    {
      return this->head_;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    Server::Server(asio::io_service& ioService)
      : ioService_(ioService),
        acceptor_(ioService_),
        socket_(ioService_)
    {

    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    Server::~Server()
    {
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void Server::listen(unsigned short port, const std::string& host)
    {
      // Open the acceptor with the option to reuse the address (i.e. SO_REUSEADDR).
      asio::ip::tcp::resolver resolver(ioService_);
      asio::ip::tcp::endpoint endpoint = *(resolver.resolve({host, std::to_string(port)}));
      acceptor_.open(endpoint.protocol());
      acceptor_.set_option(asio::ip::tcp::acceptor::reuse_address(true));
      acceptor_.bind(endpoint);
      acceptor_.listen();

      this->accept();
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void Server::accept()
    {
      acceptor_.async_accept(socket_,
        [this](std::error_code ec)
        {
          // Check whether the server was stopped by a signal before this
          // completion handler had a chance to run.
          if (!acceptor_.is_open())
          {
            return;
          }

          if (!ec)
          {
            connection_manager_.start(std::make_shared<connection>(std::move(socket_), connection_manager_, request_handler_));
          }

          this->accept();
        });
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void Server::registerHandler(const std::regex& expression, std::function<void(Server::Request& req, Server::Response& res)> handler)
    {
    }
    //----------------------------------------------------------------//
  }
}
