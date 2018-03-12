
#include "manifold/http_message.hpp"

namespace manifold
{
  namespace http
  {
    //----------------------------------------------------------------//
    message::message(http::connection& conn, std::uint32_t stream_id)
      : connection_(conn),
        stream_id_(stream_id)
    {
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    message::message(message&& source)
      : connection_(source.connection_),
      stream_id_(source.stream_id_)
    {

    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    message::~message()
    {
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    std::uint32_t message::stream_id() const
    {
      return this->stream_id_;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    ::manifold::http::version message::http_version() const
    {
      return this->connection_.version();
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    http::header_block& message::header_block()
    {
      return headers_;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void message::cancel()
    {
      this->connection_.send_reset(this->stream_id_, v2_errc::cancel);
    }
    //----------------------------------------------------------------//

//    //----------------------------------------------------------------//
//    void message::on_close(const std::function<void(const std::error_code&)>& cb)
//    {
//      if (this->connection_)
//        this->connection_->on_close(this->stream_id_, cb);
//    }
//    //----------------------------------------------------------------//

//    //----------------------------------------------------------------//
//    std::string message::errorMessage() const
//    {
//      switch (this->error_code_)
//      {
//        case error_code::SocketError: return "Socket Error";
//        case error_code::HeadCorrupt: return "Headers Are Corrupt";
//        case error_code::HeadTooLarge: return "Headers Are Too Large";
//        default: return "";
//      }
//    }
//    //----------------------------------------------------------------//
  }
}