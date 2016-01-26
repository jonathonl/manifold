#pragma once
#ifndef MANIFOLD_USER_AGENT_HPP
#define MANIFOLD_USER_AGENT_HPP

#include "uniform_resource_identifier.hpp"
#include "http_client.hpp"

#include <map>
#include <set>
#include <memory>

namespace manifold
{
  namespace http
  {
    class endpoint
    {
    public:
      endpoint() {}
      endpoint(const uri& uri)
        : host_(uri.host()), port_(uri.port()), encrypted_(uri.scheme_name() == "https")
      {
        if (!port_)
          port_ = (unsigned short)(encrypted_ ? 443 : 80);
      }
      endpoint(const std::string& host, bool encrypted = false, unsigned short port = 0)
        : host_(host), port_(port), encrypted_(encrypted)
      {
        if (!port_)
          port_ = (unsigned short)(encrypted_ ? 443 : 80);
      }
      bool operator<(const endpoint& other) const
      {
        return (this->host_ < other.host_ || this->port_ < other.port_ || (int)this->encrypted_ < (int)other.encrypted_);
      }
      const std::string& host() const { return host_; }
      unsigned short port() const { return port_; }
      bool encrypted() const { return encrypted_; }
      std::string socket_address() const
      {
        std::stringstream ret;
        ret << this->host_ << ":" << this->port_;
        return ret.str();
      }
    private:
      std::string host_;
      unsigned short port_;
      bool encrypted_;
    };

    class user_agent
    {
    public:
      user_agent(asio::io_service& ioservice)
        : io_service_(ioservice), tcp_resolver_(ioservice)
      { }
      ~user_agent()
      {
        // TODO
      }

      void make_request(const endpoint& ep, const std::function<void(const std::error_code& connect_error, client::request&& req)>& cb)
      {
        std::string socket_address = ep.host() + ":" + std::to_string(ep.port());

        auto it = this->sessions_.find(ep);
        if (it != this->sessions_.end())
        {
          if (it->second.conn)
          {
            std::uint32_t stream_id = it->second.conn->create_stream(0, 0);
            cb ? cb(std::error_code(), client::request(request_head("/", "GET", {{"user-agent", "Manifold"}}), it->second.conn, stream_id, socket_address)) : void();
          }
          else
          {
            it->second.pending_requests.push(cb);
          }
        }
        else
        {
          session& sess = this->sessions_[socket_address];
          sess.pending_requests.emplace(cb);
          sess.ep = ep;


          if (ep.encrypted())
          {

          }
          else
          {
            sess.sock = std::make_shared<manifold::non_tls_socket>(this->io_service_);
            non_tls_socket s(this->io_service_);
            sess.test.emplace(std::move(s));

            this->tcp_resolver_.async_resolve(asio::ip::tcp::resolver::query(ep.host(), std::to_string(ep.port())), [this, socket_address](const std::error_code& ec, asio::ip::tcp::resolver::iterator it)
            {
              auto sess_it = this->sessions_.find(socket_address);
              if (sess_it != this->sessions_.end())
              {
                if (ec)
                {
                  sess_it->second.process_pending_requests(ec);
                }
                else
                {
                  std::cout << it->host_name() << std::endl;
                  std::cout << it->endpoint().address().to_string() << std::endl;
                  std::cout << it->endpoint().port() << std::endl;
                  non_tls_socket& sock = dynamic_cast<non_tls_socket&>(*sess_it->second.sock);
                  ((asio::ip::tcp::socket&)sock).async_connect(*it, [this, socket_address](const std::error_code& ec)
                  {
                    auto sess_it = this->sessions_.find(socket_address);
                    if (sess_it != this->sessions_.end())
                    {
                      if (ec)
                      {
                        sess_it->second.process_pending_requests(ec);
                      }
                      else
                      {
                        sess_it->second.conn = std::make_shared<v1_connection<request_head, response_head>>(std::move(dynamic_cast<non_tls_socket&>(*sess_it->second.sock)));
                        sess_it->second.conn->on_close(std::bind(&user_agent::handle_connection_close, this, std::placeholders::_1, sess_it->second.ep.socket_address()));
                        sess_it->second.conn->run();
                        sess_it->second.process_pending_requests(std::error_code());
                      }
                    }
                  });
                }
              }
            });
          }
        }

      }
    private:
      class session
      {
      public:
        endpoint ep;
        std::shared_ptr<http::connection<request_head, response_head>> conn;
        std::shared_ptr<socket> sock;
        std::queue<std::function<void(const std::error_code& connect_error, client::request&& req)>> pending_requests;
        std::queue<non_tls_socket> test;

        void process_pending_requests(const std::error_code& ec)
        {
          if (!this->pending_requests_processed_)
          {
            this->pending_requests_processed_ = true;
            while (this->pending_requests.size())
            {
              if (this->pending_requests.front())
              {
                if (ec || !this->conn || this->conn->is_closed())
                {
                  this->pending_requests.front()(ec ? ec : std::make_error_code(std::errc::connection_aborted), client::request(request_head(), nullptr, 0, ""));
                }
                else
                {
                  std::uint32_t stream_id = this->conn->create_stream(0, 0);
                  this->pending_requests.front()(std::error_code(), client::request(request_head("/", "GET", {{"user-agent", "Manifold"}}), this->conn, stream_id, this->ep.socket_address()));
                }
              }
              this->pending_requests.pop();
            }
          }
        }
      private:
        bool pending_requests_processed_ = false;
      };

      asio::io_service& io_service_;
      asio::ip::tcp::resolver tcp_resolver_;
      std::map<endpoint, session> sessions_;

      void handle_connection_close(errc ec, const std::string& sock_addr)
      {
        this->sessions_.erase(sock_addr);
      }


    };
  }
}


#endif //MANIFOLD_USER_AGENT_HPP
