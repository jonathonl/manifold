#include <tcp.hpp>
#include "socket.hpp"
#include <iostream>
#include <cstring>
#include <algorithm>

#include "http_response_head.hpp"

using namespace std;
using namespace IPSuite;
int main()
{

  std::string name = "          my name   \t";
  std::string value = " foo";
  std::cout << "\"" << name << "\"" << std::endl;
  std::cout << "\"" << value << "\"" << std::endl;
  name.erase(0, name.find_first_not_of(" \t\f\v"));
  name.erase(name.find_last_not_of(" \t\f\v")+1);
  value.erase(0, value.find_first_not_of(" \t\f\v"));
  value.erase(value.find_last_not_of(" \t\f\v")+1);
  std::cout << "\"" << name << "\"" << std::endl;
  std::cout << "\"" << value << "\"" << std::endl;

  HTTP::ResponseHead res;
  res.statusCode(HTTP::StatusCode::Ok);



//  struct addrinfo hints, *res;
//
//  memset(&hints, 0, sizeof hints);
//  hints.ai_family = (int)Socket::Family::Unspecified;
//  hints.ai_socktype = (int)Socket::Type::Stream;
//
//  Socket s = TCP::connect(80, "rfactor.net", std::chrono::seconds(15));
//  TCP::recvLine(s, nullptr, 8);
//
//  s.close();

  return 0;
}