
#include "http_message.hpp"
#include "tcp.hpp"

namespace manifold
{
  namespace http
  {
    //----------------------------------------------------------------//
    message::message(const std::shared_ptr<http::v2_connection>& conn, std::uint32_t stream_id)
      : connection_(conn),
        stream_id_(stream_id)
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

    void message::close(http::errc error_code)
    {
      this->connection_->send_reset_stream(this->stream_id_, error_code);
    }

    //----------------------------------------------------------------//
    void message::on_close(const std::function<void(std::uint32_t ec)>& cb)
    {
      this->connection_->on_close(this->stream_id_, cb);
    }
    //----------------------------------------------------------------//

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