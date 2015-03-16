
#include <sstream>

#include "http_outgoing_message.hpp"
#include "tcp.hpp"

namespace IPSuite
{
  namespace HTTP
  {
    //----------------------------------------------------------------//
    OutgoingMessage::OutgoingMessage(MessageHead& head, Socket& sock)
      : Message(head, sock),
        bytesSent_(0),
        headersSent_(false)
    {
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    OutgoingMessage::~OutgoingMessage()
    {
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    bool OutgoingMessage::send(const char* data, std::size_t dataSize)
    {
      bool ret = true;
      if (!this->headersSent_)
      {

        std::string transferEncoding = this->head_.header("transfer-encoding");
        std::string contentLengthString = this->head_.header("content-length");

        if (contentLengthString.empty() && transferEncoding.empty())
        {
          this->transferEncoding_ = TransferEncoding::Chunked;
          this->head_.header("transfer-encoding", "chunked");
        }
        else if (contentLengthString.empty() && transferEncoding != "identity")
        {
          this->transferEncoding_ = TransferEncoding::Chunked;
        }
        else
        {
          this->transferEncoding_ = TransferEncoding::Identity;


          if (contentLengthString.empty())
          {
            // multipart/byteranges not supported
            this->head_.header("content-length", std::to_string(dataSize));
            this->contentLength_ = dataSize;
          }
          else
          {
            std::stringstream contentLengthStream(contentLengthString);
            contentLengthStream >> this->contentLength_;
          }
        }

        ret = this->sendHead();

        this->headersSent_ = true;
      }

      if (ret)
      {
        if (transferEncoding_ == TransferEncoding::Chunked)
          ret = this->sendChunkedEntity(data, dataSize);
        else
          ret = this->sendKnownLengthEntity(data, dataSize);
      }

      return ret;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    bool OutgoingMessage::sendChunkedEntity(const char* data, std::size_t dataSize)
    {
      bool ret = false;

      std::stringstream sizeLineStream;
      sizeLineStream << std::hex << dataSize;
      sizeLineStream << "\r\n";

      std::string sizeLine(sizeLineStream.str());


      if (!TCP::sendAll(this->socket_, sizeLine.data(), sizeLine.size()))
      {
        // TODO: Set error.
      }
      else
      {
        if (!TCP::sendAll(this->socket_, sizeLine.data(), sizeLine.size()))
        {
          // TODO: Set error.
        }
        else
        {
          this->bytesSent_ += dataSize;

          const char* newLine = "\r\n";
          if (!TCP::sendAll(this->socket_, newLine, 2))
          {
            // TODO: Set error.
          }
          else
          {
            ret = true;
          }
        }
      }

      return ret;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    bool OutgoingMessage::sendKnownLengthEntity(const char* data, std::size_t dataSize)
    {
      bool ret = false;

      std::size_t bytesRemaining = (this->contentLength_ - this->bytesSent_);
      if (bytesRemaining == 0)
      {
        // TODO: Set error.
      }
      else if (bytesRemaining < dataSize)
      {
        // TODO: Set error.
      }
      else
      {
        if (!TCP::sendAll(this->socket_, data, dataSize))
        {
          // TODO: Set error.
        }
        else
        {
          this->bytesSent_ += dataSize;

          if (this->bytesSent_ == this->contentLength_)
            this->eof_ = true;

          ret = true;
        }
      }

      return ret;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    bool OutgoingMessage::sendHead()
    {
      bool ret = false;

      std::string headerString;
      HTTP::MessageHead::serialize(this->head_, headerString);

      if (!TCP::sendAll(this->socket_, headerString.data(), headerString.size()))
      {
        // TODO: Set error.
      }
      else
      {
        ret = true;
      }

      return ret;
    }
    //----------------------------------------------------------------//
  }
}