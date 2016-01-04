//
//#include <unistd.h>
//#include <errno.h>
//#include <fcntl.h>
//#include <cstring>

#include "socket.hpp"

#include <assert.h>

//################################################################//
namespace manifold
{
  //----------------------------------------------------------------//
  void non_tls_socket::recv(char* data, std::size_t data_sz, std::function<void(const std::error_code& ec, std::size_t bytes_read)>&& cb)
  {
    asio::async_read(*this->s_, asio::buffer(data, data_sz), std::move(cb));
  }
  //----------------------------------------------------------------//

  //----------------------------------------------------------------//
  void non_tls_socket::recv(char* data, std::size_t data_sz, const std::function<void(const std::error_code& ec, std::size_t bytes_read)>& cb)
  {
    this->recv(data, data_sz, std::function<void(const std::error_code& ec, std::size_t bytes_read)>(cb));
  }
  //----------------------------------------------------------------//

  //----------------------------------------------------------------//
  void non_tls_socket::recvline(char* buf, std::size_t buf_sz, std::function<void(const std::error_code& ec, std::size_t bytes_read)>&& cb, const std::string& delim)
  {
    if (buf_sz < delim.size())
      cb ? cb(make_error_code(std::errc::value_too_large), 0) : void();
    else
      this->recvline(buf, buf_sz, 0, buf, std::move(cb), delim);
  }
  //----------------------------------------------------------------//

  //----------------------------------------------------------------//
  void non_tls_socket::recvline(char* buf, std::size_t buf_sz, const std::function<void(const std::error_code& ec, std::size_t bytes_read)>& cb, const std::string& delim)
  {
    this->recvline(buf, buf_sz, std::function<void(const std::error_code& ec, std::size_t bytes_read)>(cb), delim);
  }
  //----------------------------------------------------------------//

  //----------------------------------------------------------------//
  void non_tls_socket::recvline(char* buf, std::size_t buf_size, std::size_t put_position, char* buf_end, std::function<void(const std::error_code& ec, std::size_t bytes_transferred)>&& cb, const std::string& delim)
  {
    this->s_->async_receive(asio::null_buffers(), 0, [this, buf, buf_size, put_position, buf_end, delim, cb](const std::error_code& ec, std::size_t bytes_transferred) mutable
    {
      if (ec)
      {
        cb ? cb(ec, 0) : void();
      }
      else
      {
        std::error_code err;
        const size_t discard_buffer_size = 8;
        std::size_t bytes_to_read = buf_size - put_position;
        if (bytes_to_read > discard_buffer_size)
          bytes_to_read = discard_buffer_size;

        const std::size_t bytes_actually_read = this->s_->receive(asio::buffer(buf + put_position, bytes_to_read), asio::ip::tcp::socket::message_peek, err);
        if (err)
        {
          cb ? cb(ec, 0) : void();
        }
        else
        {
          assert(bytes_actually_read > 0);
          buf_end += bytes_actually_read;

          bool delim_found = false;
          char* search_result = std::search(buf, buf_end, delim.begin(), delim.end());
          if (search_result != buf_end)
          {
            buf_end = search_result + delim.size();
            delim_found = true;
          }

          std::array<char, discard_buffer_size> discard;
          size_t bytes_to_discard = buf_end - &buf[put_position];
          this->s_->receive(asio::buffer(discard.data(), bytes_to_discard), 0, err);
          put_position += bytes_to_discard;

          std::size_t buf_output_size = buf_end - buf;
          if (delim_found || err)
            cb ? cb(err, buf_output_size) : void();
          else if (buf_output_size == buf_size)
            cb ? cb(make_error_code(std::errc::value_too_large), buf_output_size) : void();
          else
            this->recvline(buf, buf_size, put_position, buf_end, std::move(cb), delim);
        }
      }
    });
  }
  //----------------------------------------------------------------//

  //----------------------------------------------------------------//
  void non_tls_socket::send(const char*const data, std::size_t data_sz, std::function<void(const std::error_code& ec, std::size_t bytes_read)>&& cb)
  {
    asio::async_write(*this->s_, asio::buffer(data, data_sz), std::move(cb));
  }
  //----------------------------------------------------------------//

  //----------------------------------------------------------------//
  void non_tls_socket::send(const char*const data, std::size_t data_sz, const std::function<void(const std::error_code& ec, std::size_t bytes_read)>& cb)
  {
    this->send(data, data_sz, std::function<void(const std::error_code& ec, std::size_t bytes_read)>(cb));
  }
  //----------------------------------------------------------------//

  //----------------------------------------------------------------//
  void non_tls_socket::close()
  {
    this->s_->close();
  }
  //----------------------------------------------------------------//

  //----------------------------------------------------------------//
  void tls_socket::recv(char* data, std::size_t data_sz, std::function<void(const std::error_code& ec, std::size_t bytes_read)>&& cb)
  {
    asio::async_read(*this->s_, asio::buffer(data, data_sz), std::move(cb));
  }
  //----------------------------------------------------------------//

  //----------------------------------------------------------------//
  void tls_socket::recv(char* data, std::size_t data_sz, const std::function<void(const std::error_code& ec, std::size_t bytes_read)>& cb)
  {
    this->recv(data, data_sz, std::function<void(const std::error_code& ec, std::size_t bytes_read)>(cb));
  }
  //----------------------------------------------------------------//

  //----------------------------------------------------------------//
  void tls_socket::recvline(char* buf, std::size_t buf_sz, std::function<void(const std::error_code& ec, std::size_t bytes_read)>&& cb, const std::string& delim)
  {
    if (buf_sz < delim.size())
      cb ? cb(make_error_code(std::errc::value_too_large), 0) : void();
    else
      this->recvline(buf, buf_sz, 0, buf, std::move(cb), delim);
  }
  //----------------------------------------------------------------//

  //----------------------------------------------------------------//
  void tls_socket::recvline(char* buf, std::size_t buf_sz, const std::function<void(const std::error_code& ec, std::size_t bytes_read)>& cb, const std::string& delim)
  {
    this->recvline(buf, buf_sz, std::function<void(const std::error_code& ec, std::size_t bytes_read)>(cb), delim);
  }
  //----------------------------------------------------------------//

  //----------------------------------------------------------------//
  void tls_socket::recvline(char* buf, std::size_t buf_size, std::size_t put_position, char* buf_end, std::function<void(const std::error_code& ec, std::size_t bytes_transferred)>&& cb, const std::string& delim)
  {
    asio::async_read(*this->s_, asio::null_buffers(), [this, buf, buf_size, put_position, buf_end, delim, cb](const std::error_code& ec, std::size_t bytes_transferred) mutable
    {
      if (ec)
      {
        cb ? cb(ec, 0) : void();
      }
      else
      {
        std::error_code err;
        const size_t discard_buffer_size = 8;
        std::size_t bytes_to_read = buf_size - put_position;
        if (bytes_to_read > discard_buffer_size)
          bytes_to_read = discard_buffer_size;

        //const std::size_t bytes_actually_read = sock.receive(asio::buffer(buf + put_position, bytes_to_read), asio::ip::tcp::socket::message_peek, err);
        int bytes_actually_read = SSL_peek(this->s_->native_handle(), buf + put_position, bytes_to_read);
        if (SSL_get_error(this->s_->native_handle(), bytes_actually_read))
        {
          cb ? cb(asio::error::make_error_code(asio::error::ssl_errors()), 0) : void();
        }
        else
        {
          assert(bytes_actually_read > 0);
          buf_end += bytes_actually_read;

          bool delim_found = false;
          char* search_result = std::search(buf, buf_end, delim.begin(), delim.end());
          if (search_result != buf_end)
          {
            buf_end = search_result + delim.size();
            delim_found = true;
          }

          std::array<char, discard_buffer_size> discard;
          size_t bytes_to_discard = buf_end - &buf[put_position];
          asio::read(*this->s_, asio::buffer(discard.data(), bytes_to_discard), err);
          put_position += bytes_to_discard;

          std::size_t buf_output_size = buf_end - buf;
          if (delim_found || err)
            cb ? cb(err, buf_output_size) : void();
          else if (buf_output_size == buf_size)
            cb ? cb(make_error_code(std::errc::value_too_large), buf_output_size) : void();
          else
            this->recvline(buf, buf_size, put_position, buf_end, std::move(cb), delim);
        }
      }
    });
  }
  //----------------------------------------------------------------//

  //----------------------------------------------------------------//
  void tls_socket::send(const char*const data, std::size_t data_sz, std::function<void(const std::error_code& ec, std::size_t bytes_read)>&& cb)
  {
    asio::async_write(*this->s_, asio::buffer(data, data_sz), std::move(cb));
  }
  //----------------------------------------------------------------//

  //----------------------------------------------------------------//
  void tls_socket::send(const char*const data, std::size_t data_sz, const std::function<void(const std::error_code& ec, std::size_t bytes_read)>& cb)
  {
    this->send(data, data_sz, std::function<void(const std::error_code& ec, std::size_t bytes_read)>(cb));
  }
  //----------------------------------------------------------------//

  //----------------------------------------------------------------//
  void tls_socket::close()
  {
    this->s_->next_layer().close();
  }
  //----------------------------------------------------------------//

//  //----------------------------------------------------------------//
//  Socket::Socket()
//    : fd_(-1), family_(Family::Unspecified), type_(Type::Unknown)
//  {
//  }
//  //----------------------------------------------------------------//
//
//  //----------------------------------------------------------------//
//  Socket::Socket(Family family, Type type, int proto)
//    : family_(family), type_(type), fd_(-1), errno_(0) //, fd_(::socket(PF_INET6, (frame_type == Type::Stream ? SOCK_STREAM : SOCK_DGRAM), 0)) // May put this in connect/listen functions if this blocks.
//  {
//    this->fd_ = ::socket((int)family, (int)type, proto);
//  }
//  //----------------------------------------------------------------//
//
//  //----------------------------------------------------------------//
//  Socket::~Socket()
//  {
//    this->close();
//  }
//  //----------------------------------------------------------------//
//
//  //----------------------------------------------------------------//
//  Socket::Socket(Socket&& source)
//  {
//    this->family_ = source.family_;
//    this->type_ = source.type_;
//    this->fd_ = source.fd_;
//    this->errno_ = source.errno_;
//    source.fd_ = -1;
//  }
//  //----------------------------------------------------------------//
//
//  //----------------------------------------------------------------//
//  Socket& Socket::operator=(Socket&& source)
//  {
//    if (this != &source)
//    {
//      this->family_ = source.family_;
//      this->type_ = source.type_;
//      this->errno_ = source.errno_;
//      this->close();
//      this->fd_ = source.fd_;
//      source.fd_ = -1;
//    }
//    return (*this);
//  }
//  //----------------------------------------------------------------//
//
//  //----------------------------------------------------------------//
//  int Socket::fd() const
//  {
//    return this->fd_;
//  }
//  //----------------------------------------------------------------//
//
//  //----------------------------------------------------------------//
//  bool Socket::isValid() const
//  {
//    return (this->fd_ != -1);
//  }
//  //----------------------------------------------------------------//
//
////  //----------------------------------------------------------------//
////  int Socket::errorCode() const
////  {
////    return this->errno_;
////  }
////  //----------------------------------------------------------------//
//
//  //----------------------------------------------------------------//
//  bool Socket::connect(const struct sockaddr* addr, socklen_t addrlen, std::int64_t microseconds)
//  {
//    bool ret = false;
//
//
//
//    int flags = fcntl(this->fd_, F_GETFL, 0);
//    fcntl(this->fd_, F_SETFL, flags | O_NONBLOCK);
//
//    int systemCallRes = ::connect(this->fd_, addr, addrlen);
//    if (systemCallRes == 0)
//    {
//      ret = true;
//    }
//    else if (errno == EINPROGRESS)
//    {
//      socklen_t len;
//      fd_set rset, wset;
//
//      FD_ZERO(&rset);
//      FD_SET(this->fd_, &rset);
//      wset = rset;
//
//      struct timeval	tval;
//
//      if( microseconds <= 0 )
//        tval.tv_sec = tval.tv_usec = 0;
//      else
//      {
//        tval.tv_sec = microseconds / 1000000;
//        tval.tv_usec = static_cast<std::int32_t>(microseconds % 1000000);
//      }
//
//      systemCallRes = ::select(this->fd_+1, &rset, &wset, nullptr, microseconds > 0 ? &tval : nullptr);
//      if (systemCallRes == 0)
//      {
//        errno = ETIMEDOUT;
//      }
//      else if (systemCallRes > 0)
//      {
//        ret = true;
//      }
//    }
//
//    fcntl(this->fd_, F_SETFL, flags);	/* restore file status flags */
//
//    if (!ret)
//      this->close();
//
//    return ret;
//
//
//
//
////    struct addrinfo hints, *res, perm;
////
////    memset(&hints, 0, sizeof hints);
////    //hints.ai_family = getSystemFamily(this->family_);
////    //hints.ai_socktype = getSystemType(this->type_);
////
////    if (getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hints, &res) == 0)
////    {
////      if (this->fd_ < 0)
////        this->fd_ = ::socket(res->ai_family, res->ai_socktype, res->ai_protocol);
////      if (this->fd_ > -1)
////      {
////        if (::connect(this->fd_, res->ai_addr, res->ai_addrlen) == 0)
////        {
////          ret = true;
////        }
////      }
////
////      memcpy(&perm, res, sizeof(addrinfo));
////
////
////      freeaddrinfo(res);
////    }
////
////    return ret;
//  }
//  //----------------------------------------------------------------//
//
//  //----------------------------------------------------------------//
//  bool Socket::listen(int backlog)
//  {
//    return (::listen(this->fd_, backlog) == 0);
//  }
//  //----------------------------------------------------------------//
//
//  //----------------------------------------------------------------//
//  bool Socket::bind(const struct sockaddr* addr, socklen_t addrlen)
//  {
//    return (::bind(this->fd_, addr, addrlen) == 0);
//  }
//  //----------------------------------------------------------------//
//
//  /*
//    char s[INET6_ADDRSTRLEN];
//    inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
//  */
//  //----------------------------------------------------------------//
//  Socket Socket::accept(struct sockaddr* addr, socklen_t* addrlen)
//  {
//    Socket ret;
//    int fd = ::accept(this->fd_, addr, addrlen);
//    if (fd)
//      ret.fd_ = fd;
//    return ret;
//  }
//  //----------------------------------------------------------------//
//
//  //----------------------------------------------------------------//
//  ssize_t Socket::send(const void *msg, std::size_t msglen, int flags)
//  {
//    return ::send(this->fd_, msg, msglen, flags);
//  }
//  //----------------------------------------------------------------//
//
//  //----------------------------------------------------------------//
//  ssize_t Socket::sendto(const void *msg, std::size_t msglen, unsigned short port, const std::string& host)
//  {
//    ssize_t ret = -1;
//    struct addrinfo hints, *res, perm;
//
//    memset(&hints, 0, sizeof hints);
//    if (getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hints, &res) == 0)
//    {
//      ret = ::sendto(this->fd_, msg, msglen, 0, res->ai_addr, res->ai_addrlen);
//    }
//    return ret;
//  }
//  //----------------------------------------------------------------//
//
//  //----------------------------------------------------------------//
//  ssize_t Socket::recv(void *buf, std::size_t len, int flags)
//  {
//    return ::recv(this->fd_, buf, len, 0);
//  }
//  //----------------------------------------------------------------//
//
//  //----------------------------------------------------------------//
//  ssize_t Socket::recvfrom(void *buf, std::size_t len, int flags, struct sockaddr *from, socklen_t *fromlen)
//  {
//    return ::recvfrom(this->fd_, buf, len, flags, from, fromlen);
//  }
//  //----------------------------------------------------------------//
//
//  //----------------------------------------------------------------//
//  void Socket::close()
//  {
//    if (this->fd_ > -1)
//    {
//      ::close(this->fd_);
//      this->fd_ = -1;
//    }
//  }
//  //----------------------------------------------------------------//
}
//################################################################//