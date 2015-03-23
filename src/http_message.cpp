
#include "http_message.hpp"
#include "tcp.hpp"

namespace IPSuite
{
  namespace HTTP
  {
    //----------------------------------------------------------------//
    Message::Message(MessageHead& head, Socket& sock)
      : head_(head),
        socket_(sock),
        transferEncoding_(TransferEncoding::Unknown),
        contentLength_(0),
        eof_(false),
        errorCode_(ErrorCode::NoError)
    {
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    Message::~Message()
    {
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    Message::ErrorCode Message::errorCode() const
    {
      return this->errorCode_;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    std::string Message::errorMessage() const
    {
      switch (this->errorCode_)
      {
        case ErrorCode::SocketError: return "Socket Error";
        case ErrorCode::HeadCorrupt: return "Headers Are Corrupt";
        case ErrorCode::HeadTooLarge: return "Headers Are Too Large";
        default: return "";
      }
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    bool Message::isGood() const
    {
      return (!this->eof_ && this->errorCode_ == ErrorCode::NoError);
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    const Socket& Message::socket() const
    {
      return this->socket_;
    }
    //----------------------------------------------------------------//
  }
}