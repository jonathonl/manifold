#include <errno.h>
#include <cstring>
#include <algorithm>

#include "tcp.hpp"

//################################################################//
namespace IPSuite
{
  //----------------------------------------------------------------//
  Socket TCP::connect(unsigned short port, const std::string& host, std::int64_t microseconds)
  {
    Socket ret;
    struct addrinfo hints, *res;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; //(int)Socket::Family::Unspecified;
    hints.ai_socktype = SOCK_STREAM; //(int)Socket::Type::Stream;

    if (getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hints, &res) == 0)
    {
      ret = Socket((Socket::Family)res->ai_family, (Socket::Type)res->ai_socktype, res->ai_protocol);
      if (ret.fd() > -1)
      {
        if (!ret.connect(res->ai_addr, res->ai_addrlen, microseconds))
        {
          ret.close();
        }
      }

      freeaddrinfo(res);
    }

    return ret;
  }
  //----------------------------------------------------------------//

  //----------------------------------------------------------------//
  Socket TCP::listen(unsigned short port, const std::string& host, int backlog)
  {
    Socket ret;

    struct addrinfo hints, *res;
    int getAddressRes;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    if (host.empty() || host == "0.0.0.0")
    {
      hints.ai_flags = AI_PASSIVE;
      getAddressRes = ::getaddrinfo(NULL, std::to_string(port).c_str(), &hints, &res);
    }
    else
    {
      getAddressRes = ::getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hints, &res);
    }

    if (getAddressRes == 0)
    {
      ret = Socket((Socket::Family)res->ai_family, (Socket::Type)res->ai_socktype, res->ai_protocol);
      if (ret.fd() > -1)
      {
        if (!ret.bind(res->ai_addr, res->ai_addrlen) || !ret.listen(backlog))
          ret.close();
      }

      ::freeaddrinfo(res);
    }


    return ret;
  }
  //----------------------------------------------------------------//

  //----------------------------------------------------------------//
  ssize_t TCP::recvLine(Socket& sock, char* buf, std::size_t bufSize, const std::string& delim)
  {
    ssize_t ret = -1;

    const size_t discardBufferSize = 1024;

    if (delim.size() <= bufSize)
    {
      bool delimFound = false;
      char* bufEnd = buf;
      std::size_t putPosition = 0;
      while (bufSize && !delimFound)
      {
        std::size_t bytesToRead = (bufSize > discardBufferSize ? discardBufferSize : bufSize);
        ssize_t bytesActuallyRead = sock.recv(&buf[putPosition], bytesToRead, MSG_PEEK);
        if (bytesActuallyRead == -1)
          break;
        bufEnd += bytesActuallyRead;
        char* searchResult = std::search(buf, bufEnd, delim.begin(), delim.end());
        if (searchResult != bufEnd)
        {
          bufEnd = searchResult + delim.size();
          delimFound = true;
        }

        char tmp[discardBufferSize];
        size_t bytesToDiscard = bufEnd - &buf[putPosition];
        sock.recv(tmp, bytesToDiscard); //TODO: Handle Error Case
        bufSize -= bytesToDiscard;
        putPosition += bytesToDiscard; //= bufEnd - buf;
      }

      if (delimFound)
        ret = putPosition;
    }
    return ret;
  }
  //----------------------------------------------------------------//

  //----------------------------------------------------------------//
  bool TCP::sendAll(Socket& sock, const char* buf, std::size_t bufSize)
  {
    bool ret = false;
    std::size_t localBytesSent = 0;
    while (localBytesSent < bufSize)
    {
      ssize_t sendResult = sock.send(&buf[localBytesSent], bufSize - localBytesSent);
      if (sendResult < 0)
      {
        break;
      }
      else
      {
        localBytesSent += sendResult;
      }
    }

    if (localBytesSent == bufSize)
      ret = true;
    return ret;
  }
  //----------------------------------------------------------------//
}
//################################################################//