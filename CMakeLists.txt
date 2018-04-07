cmake_minimum_required(VERSION 2.8.4)
project(manifold)

#set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++2a -fcoroutines-ts -stdlib=libc++")

#set(CMAKE_MACOSX_RPATH 1)

#find_path(MYSQL_INCLUDE_DIR
#          NAMES "mysql.h"
#          PATHS
#          "/usr/local/mysql/include"
#          "/usr/include/mysql"
#          "/usr/local/include/mysql"
#          "/usr/mysql/include/mysql")
#
#find_library(MYSQL_LIBRARY
#             NAMES "mysqlclient"
#             PATHS
#             "/usr/local/mysql/lib"
#             "/lib/mysql"
#             "/lib64/mysql"
#             "/usr/lib/mysql"
#             "/usr/lib64/mysql"
#             "/usr/local/lib/mysql"
#             "/usr/local/lib64/mysql"
#             "/usr/mysql/lib/mysql"
#             "/usr/mysql/lib64/mysql")

find_package(OpenSSL REQUIRED)
find_package(Boost
             COMPONENTS coroutine
             REQUIRED)

add_definitions(-DASIO_STANDALONE -DASIO_HAS_MOVE -DBOOST_COROUTINES_NO_DEPRECATION_WARNING)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-unused-local-typedefs -DASIO_STANDALONE -DASIO_HAS_MOVE -std=c++1y")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -DASIO_STANDALONE -std=c++11 -stdlib=libc++")

#set(CMAKE_CXX_COMPILER "/opt/local/bin/g++-mp-4.9")
#set(CMAKE_CC_COMPILER "/opt/local/bin/gcc-mp-4.9")

#set(HEADER_FILES
#    include/socket.hpp
#    include/tcp.hpp
#    include/uniform_resource_identifier.hpp
#    include/http_error_category.hpp
#    include/http_frame.hpp
#    include/http_v2_connection.hpp
#    include/http_v2_message_head.hpp
#    include/http_request_head.hpp
#    include/http_v2_response_head.hpp
#    include/http_message.hpp
#    include/http_response_head.hpp
#    include/http_incoming_message.hpp
#    include/http_outgoing_message.hpp
#    include/http_client.hpp
#    include/http_server.hpp
#    include/http_router.hpp
#    include/hpack.hpp
#    include/http_v1_connection.hpp
#    include/http_v1_message_head.hpp
#    include/http_v1_request_head.hpp
#    include/http_v2_request_head.hpp
#    include/http_message_head.hpp
#    include/http_v1_response_head.hpp
#    include/http_stream_client.hpp
#    include/http_file_transfer.hpp)
#    #include/mysql.hpp)
#
#set(SOURCE_FILES
#    src/socket.cpp
#    src/tcp.cpp
#    src/uniform_resource_identifier.cpp
#    src/http_error_category.cpp
#    src/http_frame.cpp
#    src/http_v2_connection.cpp
#    src/http_v2_message_head.cpp
#    src/http_request_head.cpp
#    src/http_v2_response_head.cpp
#    src/http_message.cpp
#    src/http_incoming_message.cpp
#    src/http_outgoing_message.cpp
#    src/http_client.cpp
#    src/http_server.cpp
#    src/http_router.cpp
#    src/hpack.cpp
#    src/http_v1_connection.cpp
#    src/http_v1_message_head.cpp
#    src/http_v1_response_head.cpp
#    src/http_response_head.cpp
#    src/http_v2_request_head.cpp
#    src/http_v1_request_head.cpp
#    src/http_message_head.cpp
#    src/http_stream_client.cpp
#    src/http_file_transfer.cpp)


add_library(manifold
            src/manifold/hpack.cpp include/manifold/hpack.hpp
            src/manifold/http_client.cpp include/manifold/http_client.hpp
            src/manifold/http_connection.cpp include/manifold/http_connection.hpp
            src/manifold/http_error_category.cpp include/manifold/http_error_category.hpp
            src/manifold/http_file_transfer.cpp include/manifold/http_file_transfer.hpp
            src/manifold/http_frame.cpp include/manifold/http_frame.hpp
            src/manifold/http_header_block.cpp include/manifold/http_header_block.hpp
            src/manifold/http_incoming_message.cpp include/manifold/http_incoming_message.hpp
            src/manifold/http_message.cpp include/manifold/http_message.hpp
            src/manifold/http_outgoing_message.cpp include/manifold/http_outgoing_message.hpp
            src/manifold/http_response_head.cpp include/manifold/http_response_head.hpp
            src/manifold/http_request_head.cpp include/manifold/http_request_head.hpp
            src/manifold/http_router.cpp include/manifold/http_router.hpp
            src/manifold/http_server.cpp include/manifold/http_server.hpp
            src/manifold/http_stream_client.cpp include/manifold/http_stream_client.hpp
            src/manifold/socket.cpp include/manifold/socket.hpp
            src/manifold/uniform_resource_identifier.cpp include/manifold/uniform_resource_identifier.hpp)

target_include_directories(manifold PUBLIC include ${OPENSSL_INCLUDE_DIR} ${Boost_INCLUDE_DIRS})
target_link_libraries(manifold ${OPENSSL_LIBRARIES} ${Boost_LIBRARIES})


add_executable(manifold-test tests/test.cpp) # ${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries(manifold-test manifold)
#target_link_libraries(manifold /opt/local/lib/libboost_system-mt.a)