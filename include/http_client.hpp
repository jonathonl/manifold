//#pragma once
//
//#ifndef IPSUITE_HTTP_CLIENT_HPP
//#define IPSUITE_HTTP_CLIENT_HPP
//
//#include <cstdint>
//#include <string>
//#include <map>
//#include <functional>
//#include <mutex>
//
//#include "http_request_head.hpp"
//#include "http_response_head.hpp"
//#include "http_outgoing_message.hpp"
//#include "http_incoming_message.hpp"
//
//namespace manifold
//{
//  namespace http
//  {
//    //================================================================//
//    class Client
//    {
//    public:
//      //================================================================//
//      class ConnectionHandle
//      {
//        friend class Client;
//      private:
//        std::uint64_t intVal_;
//        ConnectionHandle(std::uint64_t intVal);
//      public:
//        ConnectionHandle();
//        ConnectionHandle(const ConnectionHandle& source);
//        ConnectionHandle& operator=(const ConnectionHandle& source);
//      };
//      //================================================================//
//
//      //================================================================//
//      class AsyncConnectionHandle
//      {
//        friend class Client;
//      private:
//        std::uint64_t intVal_;
//        AsyncConnectionHandle(std::uint64_t intVal);
//      public:
//        AsyncConnectionHandle();
//        AsyncConnectionHandle(const ConnectionHandle& source);
//        AsyncConnectionHandle& operator=(const ConnectionHandle& source);
//      };
//      //================================================================//
//
//      //================================================================//
//      class request : public outgoing_message
//      {
//      private:
//        request_head head_;
//      public:
//        request(request_head&& head, Socket& sock);
//        request();
//
//        request_head& head();
//      };
//      //================================================================//
//
//      //================================================================//
//      class response : public incoming_message
//      {
//      private:
//        response_head head_;
//      public:
//        response(response_head&& head, Socket& sock);
//        response();
//
//        const response_head& head() const;
//      };
//      //================================================================//
//    private:
//      std::mutex connectionMapMtx_;
//      std::uint64_t connectionHandleLastVal_;
//      std::map<ConnectionHandle,Socket> connectionMap_;
//      std::list<AsyncConnectionHandle> asyncConnectionList_;
//    public:
//      Client();
//      ~Client();
//    public:
//      ConnectionHandle connect(std::string& host, std::uint16_t port = 0);
//      AsyncConnectionHandle asyncConnect(std::string& host, std::uint16_t port = 0);
//
//      void request(ConnectionHandle handle, std::function<void(request& request)>&& requestFn, std::function<void(response& response)>&& responseFn);
//      void request(AsyncConnectionHandle handle, std::function<void(request& request)>&& requestFn, std::function<void(response& response)>&& responseFn);
//
//      void close(ConnectionHandle handle);
//      void close(AsyncConnectionHandle handle);
//    };
//    //================================================================//
//  }
//}
//
//#endif // IPSUITE_HTTP_CLIENT_HPP