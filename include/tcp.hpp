#pragma once

#ifndef IPSUITE_TCP_HPP
#define IPSUITE_TCP_HPP

#include "socket.hpp"

//################################################################//
namespace IPSuite
{
  //================================================================//
  namespace TCP
  {
    //----------------------------------------------------------------//
    Socket connect(unsigned short port, const std::string& host, std::int64_t microseconds = 0);
    template< class Rep, class Period>
    Socket connect(unsigned short port, const std::string& host, const std::chrono::duration<Rep,Period>& timeout = 0)
    {
      return TCP::connect(port, host, std::chrono::duration_cast<std::chrono::microseconds>(timeout).count());
    }
    Socket listen(unsigned short port, const std::string& host = "", int backlog = SOMAXCONN);
    ssize_t recvLine(Socket& sock, char* buf, std::size_t bufSize, const std::string& delim = "\r\n");

    // I may make this a Socket member in the future since socket errors are the only case where it would fail.
    bool sendAll(Socket& sock, const char* buf, std::size_t bufSize);
    //----------------------------------------------------------------//
  };
  //================================================================//
}
//################################################################//

#endif // IPSUITE_TCP_HPP