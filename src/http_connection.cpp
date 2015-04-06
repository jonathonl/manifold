
#include <iostream>

#include "http_connection.hpp"

namespace manifold
{
  namespace http
  {
    //----------------------------------------------------------------//
    connection::connection(asio::ip::tcp::socket&& sock)
    : socket_(std::move(sock))
    {
      std::cout << this->socket_.non_blocking() << ":" __FILE__ << "/" << __LINE__ << std::endl;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    connection::~connection()
    {
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void connection::on_new_stream(const std::function<void(std::int32_t stream_id, std::list<std::pair<std::string,std::string>>&& headers, std::int32_t stream_dependency_id)>& fn)
    {
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void connection::on_close(const std::function<void()>& fn)
    {
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void connection::on_data_frame(std::int32_t stream_id, const std::function<void(const char* const buf, std::size_t buf_size)>& fn)
    {
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void connection::on_end_frame(std::int32_t stream_id, const std::function<void()>& fn)
    {
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void connection::on_window_update(std::int32_t stream_id, const std::function<void()>& fn)
    {
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    bool connection::send_headers_frame(std::int32_t stream_id, const message_head& head)
    {
      return false;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    bool connection::send_data_frame(std::int32_t stream_id, const char*const data, std::size_t data_sz)
    {
      return false;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void connection::send_end_frame(std::int32_t stream_id)
    {
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void connection::send_end_frame(std::int32_t stream_id, const char*const data, std::size_t data_sz)
    {
    }
    //----------------------------------------------------------------//

//    //----------------------------------------------------------------//
//    void connection::send(char* buf, std::size_t buf_size, const std::function<void(const std::error_code& ec, std::size_t bytes_transferred)>& handler)
//    {
//      this->socket_.async_send(asio::buffer(buf, buf_size), handler);
//    }
//    //----------------------------------------------------------------//
//
//    //----------------------------------------------------------------//
//    void connection::recv(const char* buf, std::size_t buf_size, const std::function<void(const std::error_code& ec, std::size_t bytes_transferred)>& handler)
//    {
//      this->socket_.async_send(asio::buffer(buf, buf_size), handler);
//    }
//    //----------------------------------------------------------------//
//
//    //----------------------------------------------------------------//
//    void connection::recvMessageHead()
//    {
//      std::shared_ptr<connection> self = this->shared_from_this();
//
//
//      TCP::recvline(this->socket_, this->incomingHeadBuffer_.data(), this->incomingHeadBuffer_.size(), [self](const std::error_code& ec, std::size_t bytes_transferred)
//      {
//        if (ec)
//        {
//          std::cout << ec.message() << ":" __FILE__ << "/" << __LINE__ << std::endl;
//        }
//        else
//        {
//
//          http::request_head requestHead;
//          http::message_head::deserialize(std::string(self->incomingHeadBuffer_.data(), bytes_transferred), requestHead);
//
//          std::cout << requestHead.url() << ":" __FILE__ << "/" << __LINE__ << std::endl;
//
//          this->requests_.emplace(std::make_shared<http::server::request>(requestHead, this->socket_));
//        }
//      }, "\r\n\r\n");
//    }
//    //----------------------------------------------------------------//
//
//    //----------------------------------------------------------------//
//    void connection::close()
//    {
//      this->socket_.close();
//      this->server_.httpSessions_.erase(std::shared_ptr<Session>(this));
//    }
//    //----------------------------------------------------------------//
  }
}