#pragma once

#ifndef IPSUITE_TCP_HPP
#define IPSUITE_TCP_HPP

#include "socket.hpp"

//################################################################//
namespace IPSuite
{
  //================================================================//
  class TCP
  {
  private:
    TCP() = delete;
  public:
    //----------------------------------------------------------------//
    static Socket connect(unsigned short port, const std::string& host, std::int64_t microseconds = 0);
    template< class Rep, class Period>
    static Socket connect(unsigned short port, const std::string& host, const std::chrono::duration<Rep,Period>& timeout = 0)
    {
      return TCP::connect(port, host, std::chrono::duration_cast<std::chrono::microseconds>(timeout).count());
    }
    static Socket listen(unsigned short port, const std::string& host = "", int backlog = SOMAXCONN);
    static ssize_t recvLine(Socket& sock, char* buf, std::size_t numBytes, const std::string& delim = "\r\n");
    //----------------------------------------------------------------//
  };
  //================================================================//
}
//################################################################//

#endif // IPSUITE_TCP_HPP