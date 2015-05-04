#include <array>
#include <string>
#include <iostream>
#include <thread>

#include "asio.hpp"
#include "http_server.hpp"

asio::io_service ioservice;
asio::ip::tcp::resolver resolv{ioservice};
asio::ip::tcp::socket tcp_socket{ioservice};
std::array<char, 4096> bytes;

void read_handler(const std::error_code &ec,
  std::size_t bytes_transferred)
{
  if (!ec)
  {
    std::cout.write(bytes.data(), bytes_transferred);
    tcp_socket.async_read_some(asio::buffer(bytes), read_handler);
  }
}

void connect_handler(const std::error_code &ec)
{
  if (!ec)
  {
    std::string r =
      "GET / http/1.1\r\nHost: theboostcpplibraries.com\r\n\r\n";
    asio::write(tcp_socket, asio::buffer(r));
    tcp_socket.async_read_some(asio::buffer(bytes), read_handler);
  }
}

void resolve_handler(const std::error_code &ec,
  asio::ip::tcp::resolver::iterator it)
{
  if (!ec)
    tcp_socket.async_connect(*it, connect_handler);
}

int main()
{


  manifold::http::server srv(ioservice);
  srv.listen(8080, "0.0.0.0");

  auto i = ioservice.run();

//  for (int i = 0; i < 15; ++i)
//  {
//
//    std::this_thread::sleep_for(std::chrono::seconds(1));
//  }
}








//using namespace std;
//using namespace manifold;
//int main()
//{
//  boost::asio::io_service io_service;
//
//  for( int x = 0; x < 42; ++x )
//  {
//    io_service.poll();
//    std::cout << "Counter: " << x << std::endl;
//
//
//  }
//
//  return 0;
//
//
////  std::string name = "          my name   \t";
////  std::string value = " foo";
////  std::cout << "\"" << name << "\"" << std::endl;
////  std::cout << "\"" << value << "\"" << std::endl;
////  name.erase(0, name.find_first_not_of(" \t\f\v"));
////  name.erase(name.find_last_not_of(" \t\f\v")+1);
////  value.erase(0, value.find_first_not_of(" \t\f\v"));
////  value.erase(value.find_last_not_of(" \t\f\v")+1);
////  std::cout << "\"" << name << "\"" << std::endl;
////  std::cout << "\"" << value << "\"" << std::endl;
////
////  http::response_head res;
////  res.status_code(http::status_code::Ok);
//
//
//
////  struct addrinfo hints, *res;
////
////  memset(&hints, 0, sizeof hints);
////  hints.ai_family = (int)Socket::Family::Unspecified;
////  hints.ai_socktype = (int)Socket::Type::Stream;
////
////  Socket s = TCP::connect(80, "rfactor.net", std::chrono::seconds(15));
////  TCP::recvline(s, nullptr, 8);
////
////  s.close();
//
//  return 0;
//}