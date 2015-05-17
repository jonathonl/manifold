
#include "http_message.hpp"
#include "tcp.hpp"

namespace manifold
{
  namespace http
  {
    //----------------------------------------------------------------//
    message::message(const std::shared_ptr<http::connection>& conn, std::uint32_t stream_id)
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

    //----------------------------------------------------------------//
    void message::on_stream_reset(const std::function<void(const std::error_code& ec)>& cb)
    {
      this->connection_->on_rst_stream_frame(this->stream_id_, cb);
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