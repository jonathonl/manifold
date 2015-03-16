
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
        eof_(false)
    {
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    Message::~Message()
    {
    }
    //----------------------------------------------------------------//
  }
}