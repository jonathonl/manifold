#pragma once

#ifndef IPSUITE_SOCKET_HPP
#define IPSUITE_SOCKET_HPP

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <cstdint>
#include <string>
#include <chrono>

//################################################################//
namespace IPSuite
{
  //================================================================//
  class Socket
  {
  public:
    //----------------------------------------------------------------//
    enum class Type
    {
      Unknown = 0,
      Raw = SOCK_RAW,
      Stream = SOCK_STREAM,
      Datagram = SOCK_DGRAM,
#ifdef SOCK_RDM
      ReliablyDeliveredMessage = SOCK_RDM,
#endif
      SequencePacketStream = SOCK_SEQPACKET
    };
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    enum class Family
    {
      Unspecified = AF_UNSPEC  , /* unspecified */
      Unix = AF_UNIX           , /* local to host (pipes, portals) */
      IPV4 = AF_INET           , /* internetwork: UDP, TCP, etc. */
      IMPLink = AF_IMPLINK     , /* arpanet imp addresses */
      Pup= AF_PUP              , /* pup protocols: e.g. BSP */
      Chaos = AF_CHAOS         , /* mit CHAOS protocols */
      XeroxNS = AF_NS          , /* XEROX NS protocols */
      ISO = AF_ISO             , /* ISO protocols */
      ECMA = AF_ECMA           , /* european computer manufacturers */
      DataKit = AF_DATAKIT     , /* datakit protocols */
      CCITT = AF_CCITT         , /* CCITT protocols, X.25 etc */
      SNA = AF_SNA             , /* IBM SNA */
      DECnet = AF_DECnet       , /* DECnet */
      DLI = AF_DLI             , /* DEC Direct data link interface */
      LAT = AF_LAT             , /* LAT */
      HyLink = AF_HYLINK       , /* NSC Hyperchannel */
      AppleTalk = AF_APPLETALK , /* Apple Talk */
      Route = AF_ROUTE         , /* Internal Routing Protocol */
      Link = AF_LINK           , /* Link layer interface */
      IPV6 = AF_INET6          , /* ipv6 */
      Max = AF_MAX
    };
    //----------------------------------------------------------------//
  private:
    //----------------------------------------------------------------//
    Family family_;
    Type type_;
    int errno_;
    int fd_;
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    Socket(const Socket& source) = delete;
    Socket& operator=(const Socket& source) = delete;
    //----------------------------------------------------------------//
  public:
    //----------------------------------------------------------------//
    Socket();
    Socket(Family fam, Type type, int proto = 0);
    Socket(Socket&& source);
    Socket& operator=(Socket&& source);
    virtual ~Socket();
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    int fd() const;
    bool isValid() const;
    //int errorCode() const;
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    bool connect(const struct sockaddr* addr, socklen_t len, std::int64_t microseconds = 0);
    template<class Rep, class Period>
    bool connect(const struct sockaddr* addr, socklen_t len, const std::chrono::duration<Rep,Period>& timeout = 0)
    {
      return this->connect(addr, len, std::chrono::duration_cast<std::chrono::microseconds>(timeout).count());
    }
    bool listen(int backlog);
    bool bind(const struct sockaddr* addr, socklen_t addrlen);
    Socket accept(struct sockaddr* addr, socklen_t* addrlen);
    ssize_t send(const void *msg, std::size_t msglen, int flags = 0);
    ssize_t sendto(const void *msg, std::size_t msglen, unsigned short port, const std::string& host);
    ssize_t recv(void *buf, std::size_t len, int flags = 0);
    ssize_t recvfrom(void *buf, std::size_t len, int flags, struct sockaddr *from, socklen_t *fromlen);
    void close();
    //----------------------------------------------------------------//
  };
  //================================================================//
}
//################################################################//

#endif // IPSUITE_SOCKET_HPP