cmake_minimum_required(VERSION 2.8.4)
project(ipsuite)

include_directories("include")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -DASIO_STANDALONE -std=c++11 -stdlib=libc++")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -DASIO_STANDALONE -std=c++11 -fno-weak")

#set(CMAKE_CXX_COMPILER "/opt/local/bin/g++-mp-4.9")
#set(CMAKE_CC_COMPILER "/opt/local/bin/gcc-mp-4.9")

set(HEADER_FILES
  include/socket.hpp
  include/tcp.hpp
  include/http_frame.hpp
  include/http_connection.hpp
  include/http_message_head.hpp
  include/http_request_head.hpp
  include/http_response_head.hpp
  include/http_message.hpp
  include/http_incoming_message.hpp
  include/http_outgoing_message.hpp
  include/http_client.hpp
  include/http_server.hpp)

set(SOURCE_FILES
  src/socket.cpp
  src/tcp.cpp
  src/http_frame.cpp
  src/http_connection.cpp
  src/http_message_head.cpp
  src/http_request_head.cpp
  src/http_response_head.cpp
  src/http_message.cpp
  src/http_incoming_message.cpp
  src/http_outgoing_message.cpp
  src/http_client.cpp
  src/http_server.cpp
  main.cpp)

add_executable(ipsuite ${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries(ipsuite /opt/local/lib/libboost_system-mt.a)