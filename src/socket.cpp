
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <cstring>

#include "socket.hpp"

//################################################################//
namespace IPSuite
{
  //----------------------------------------------------------------//
  Socket::Socket()
    : fd_(-1), family_(Family::Unspecified), type_(Type::Unknown)
  {
  }
  //----------------------------------------------------------------//

  //----------------------------------------------------------------//
  Socket::Socket(Family family, Type type, int proto)
    : family_(family), type_(type), fd_(-1), errno_(0) //, fd_(::socket(PF_INET6, (type == Type::Stream ? SOCK_STREAM : SOCK_DGRAM), 0)) // May put this in connect/listen functions if this blocks.
  {
    this->fd_ = ::socket((int)family, (int)type, proto);
  }
  //----------------------------------------------------------------//

  //----------------------------------------------------------------//
  Socket::~Socket()
  {
    this->close();
  }
  //----------------------------------------------------------------//

  //----------------------------------------------------------------//
  Socket::Socket(Socket&& source)
  {
    this->family_ = source.family_;
    this->type_ = source.type_;
    this->fd_ = source.fd_;
    this->errno_ = source.errno_;
    source.fd_ = -1;
  }
  //----------------------------------------------------------------//

  //----------------------------------------------------------------//
  Socket& Socket::operator=(Socket&& source)
  {
    if (this != &source)
    {
      this->family_ = source.family_;
      this->type_ = source.type_;
      this->errno_ = source.errno_;
      this->close();
      this->fd_ = source.fd_;
      source.fd_ = -1;
    }
    return (*this);
  }
  //----------------------------------------------------------------//

  //----------------------------------------------------------------//
  int Socket::fd() const
  {
    return this->fd_;
  }
  //----------------------------------------------------------------//

//  //----------------------------------------------------------------//
//  int Socket::errorCode() const
//  {
//    return this->errno_;
//  }
//  //----------------------------------------------------------------//

  //----------------------------------------------------------------//
  bool Socket::connect(const struct sockaddr* addr, socklen_t addrlen, std::int64_t microseconds)
  {
    bool ret = false;



    int flags = fcntl(this->fd_, F_GETFL, 0);
    fcntl(this->fd_, F_SETFL, flags | O_NONBLOCK);

    int systemCallRes = ::connect(this->fd_, addr, addrlen);
    if (systemCallRes == 0)
    {
      ret = true;
    }
    else if (errno == EINPROGRESS)
    {
      socklen_t len;
      fd_set rset, wset;

      FD_ZERO(&rset);
      FD_SET(this->fd_, &rset);
      wset = rset;

      struct timeval	tval;

      if( microseconds <= 0 )
        tval.tv_sec = tval.tv_usec = 0;
      else
      {
        tval.tv_sec = microseconds / 1000000;
        tval.tv_usec = static_cast<std::int32_t>(microseconds % 1000000);
      }

      systemCallRes = ::select(this->fd_+1, &rset, &wset, nullptr, microseconds > 0 ? &tval : nullptr);
      if (systemCallRes == 0)
      {
        errno = ETIMEDOUT;
      }
      else if (systemCallRes > 0)
      {
        ret = true;
      }
    }

    fcntl(this->fd_, F_SETFL, flags);	/* restore file status flags */

    if (!ret)
      this->close();

    return ret;




//    struct addrinfo hints, *res, perm;
//
//    memset(&hints, 0, sizeof hints);
//    //hints.ai_family = getSystemFamily(this->family_);
//    //hints.ai_socktype = getSystemType(this->type_);
//
//    if (getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hints, &res) == 0)
//    {
//      if (this->fd_ < 0)
//        this->fd_ = ::socket(res->ai_family, res->ai_socktype, res->ai_protocol);
//      if (this->fd_ > -1)
//      {
//        if (::connect(this->fd_, res->ai_addr, res->ai_addrlen) == 0)
//        {
//          ret = true;
//        }
//      }
//
//      memcpy(&perm, res, sizeof(addrinfo));
//
//
//      freeaddrinfo(res);
//    }
//
//    return ret;
  }
  //----------------------------------------------------------------//

  //----------------------------------------------------------------//
  bool Socket::listen(int backlog)
  {
    return (::listen(this->fd_, backlog) == 0);
  }
  //----------------------------------------------------------------//

  //----------------------------------------------------------------//
  bool Socket::bind(const struct sockaddr* addr, socklen_t addrlen)
  {
    return (::bind(this->fd_, addr, addrlen) == 0);
  }
  //----------------------------------------------------------------//

  /*
    char s[INET6_ADDRSTRLEN];
    inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
  */
  //----------------------------------------------------------------//
  Socket Socket::accept(struct sockaddr* addr, socklen_t* addrlen)
  {
    Socket ret;
    int fd = ::accept(this->fd_, addr, addrlen);
    if (fd)
      ret.fd_ = fd;
    return ret;
  }
  //----------------------------------------------------------------//

  //----------------------------------------------------------------//
  ssize_t Socket::send(const void *msg, std::size_t msglen, int flags)
  {
    return ::send(this->fd_, msg, msglen, flags);
  }
  //----------------------------------------------------------------//

  //----------------------------------------------------------------//
  ssize_t Socket::sendto(const void *msg, std::size_t msglen, unsigned short port, const std::string& host)
  {
    ssize_t ret = -1;
    struct addrinfo hints, *res, perm;

    memset(&hints, 0, sizeof hints);
    if (getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hints, &res) == 0)
    {
      ret = ::sendto(this->fd_, msg, msglen, 0, res->ai_addr, res->ai_addrlen);
    }
    return ret;
  }
  //----------------------------------------------------------------//

  //----------------------------------------------------------------//
  ssize_t Socket::recv(void *buf, std::size_t len, int flags)
  {
    return ::recv(this->fd_, buf, len, 0);
  }
  //----------------------------------------------------------------//

  //----------------------------------------------------------------//
  ssize_t Socket::recvfrom(void *buf, std::size_t len, int flags, struct sockaddr *from, socklen_t *fromlen)
  {
    return ::recvfrom(this->fd_, buf, len, flags, from, fromlen);
  }
  //----------------------------------------------------------------//

  //----------------------------------------------------------------//
  void Socket::close()
  {
    if (this->fd_ > -1)
    {
      ::close(this->fd_);
      this->fd_ = -1;
    }
  }
  //----------------------------------------------------------------//
}
//################################################################//