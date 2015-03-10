
#include "http_incoming_message.hpp"

namespace IPSuite
{
  namespace HTTP
  {
    //----------------------------------------------------------------//
    IncomingMessage::IncomingMessage(MessageHead& head, Socket&& sock)
      : head_(head), transferEncoding_(TransferEncoding::Unknown)
    {
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    IncomingMessage::~IncomingMessage()
    {
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    ssize_t IncomingMessage::recv(char* buff, std::size_t buffSize)
    {



    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    ssize_t IncomingMessage::recvChunkedEntity(char* buff, std::size_t buffSize)
    {
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    ssize_t IncomingMessage::recvKnownLengthEntity(char* buff, std::size_t buffSize)
    {
    }
    //----------------------------------------------------------------//
  }
}