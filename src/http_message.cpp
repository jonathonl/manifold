
#include "http_message.hpp"
#include "tcp.hpp"

namespace manifold
{
  namespace http
  {
    //----------------------------------------------------------------//
    message::message(message_head& head, const std::shared_ptr<http::connection>& conn, std::int32_t stream_id)
      : head_(head),
        connection_(conn),
        stream_id_(stream_id),
        transfer_encoding_(transfer_encoding::Unknown),
        content_length_(0)
    {
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    message::~message()
    {
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