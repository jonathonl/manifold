//#include "tcp.hpp"
//#include "http_client.hpp"
//
//namespace manifold
//{
//  namespace http
//  {
//    //----------------------------------------------------------------//
//    Client::request::request(request_head&& head, Socket& sock)
//      : outgoing_message(this->head_, sock)
//    {
//      this->head_ = std::move(head);
//    }
//    //----------------------------------------------------------------//
//
//    //----------------------------------------------------------------//
//    Client::request::request()
//    {
//    }
//    //----------------------------------------------------------------//
//
//    //----------------------------------------------------------------//
//    request_head& Client::request::head()
//    {
//      return this->head_;
//    }
//    //----------------------------------------------------------------//
//
//    //----------------------------------------------------------------//
//    Client::response::response(response_head&& head, Socket& sock)
//      : incoming_message(this->head_, sock)
//    {
//      this->head_ = std::move(head);
//    }
//    //----------------------------------------------------------------//
//
//    //----------------------------------------------------------------//
//    Client::response::response()
//    {
//
//    }
//    //----------------------------------------------------------------//
//
//    //----------------------------------------------------------------//
//    const response_head& Client::response::head() const
//    {
//      return this->head_;
//    }
//    //----------------------------------------------------------------//
//
//    //----------------------------------------------------------------//
//    Client::Client()
//    {
//      this->connectionHandleLastVal_ = 0;
//    }
//    //----------------------------------------------------------------//
//
//    //----------------------------------------------------------------//
//    Client::~Client()
//    {
//    }
//    //----------------------------------------------------------------//
//
//    //----------------------------------------------------------------//
//    Client::ConnectionHandle Client::connect(std::string& host, std::uint16_t port)
//    {
//      ConnectionHandle ret;
////      if (!port)
////        port = 80;
////      Socket sock = TCP::connect(port, host);
////      if (sock.isValid())
////      {
////        std::lock_guard<std::mutex> lk(this->connectionMapMtx_);
////        this->connectionHandleLastVal_++;
////        if (this->connectionHandleLastVal_ == 0)
////          this->connectionHandleLastVal_++;
////        auto insertResult = this->connectionMap_.emplace(ConnectionHandle(this->connectionHandleLastVal_), std::move(sock));
////        if (insertResult.second)
////          ret = ConnectionHandle(this->connectionHandleLastVal_);
////      }
//
//      return ret;
//    }
//    //----------------------------------------------------------------//
//
//    //----------------------------------------------------------------//
//    Client::AsyncConnectionHandle Client::asyncConnect(std::string& host, std::uint16_t port)
//    {
//    }
//    //----------------------------------------------------------------//
//
//    //----------------------------------------------------------------//
//    void Client::request(ConnectionHandle handle, std::function<void(request& request)>&& requestFn, std::function<void(response& response)>&& responseFn)
//    {
////      bool found = false;
////      Socket sock;
////
////      {
////        std::lock_guard<std::mutex> lk(this->connectionMapMtx_);
////        auto it = this->connectionMap_.find(handle);
////        if (it != this->connectionMap_.end())
////        {
////         sock = std::move(it.second);
////        }
////
////      }
////
////
////
////
////      {
////        std::lock_guard<std::mutex> lk(this->connectionMapMtx_);
////        auto it = this->connectionMap_.find(handle);
////        if (it != this->connectionMap_.end())
////        {
////          it.second = std::move(sock);
////        }
////      }
//    }
//    //----------------------------------------------------------------//
//
//    //----------------------------------------------------------------//
//    void Client::request(AsyncConnectionHandle handle, std::function<void(request& request)>&& requestFn, std::function<void(response& response)>&& responseFn)
//    {
//    }
//    //----------------------------------------------------------------//
//
//    //----------------------------------------------------------------//
//    void Client::close(ConnectionHandle handle)
//    {
//    }
//    //----------------------------------------------------------------//
//
//    //----------------------------------------------------------------//
//    void Client::close(AsyncConnectionHandle handle)
//    {
//    }
//  }
//}
