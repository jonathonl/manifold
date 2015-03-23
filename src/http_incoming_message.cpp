
#include <sstream>
#include <algorithm>

#include "http_incoming_message.hpp"
#include "tcp.hpp"

namespace IPSuite
{
  namespace HTTP
  {
    //----------------------------------------------------------------//
    IncomingMessage::IncomingMessage(MessageHead& head, Socket& sock)
      : Message(head, sock), bytesReceived_(0), bytesRemainingInChunk_(0)
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
      ssize_t ret = -1;
      if (this->transferEncoding_ == TransferEncoding::Chunked)
        ret = this->recvChunkedEntity(buff, buffSize);
      else
        ret = this->recvKnownLengthEntity(buff, buffSize);
      return ret;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    ssize_t IncomingMessage::recvChunkedEntity(char* buff, std::size_t buffSize)
    {
      ssize_t ret = -1;

      if (!this->eof_)
      {
        if (bytesRemainingInChunk_ == 0)
        {
          std::string sizeLine(2048, '\0');
          ssize_t recvResult = TCP::recvLine(this->socket_, &sizeLine[0], 2048, "\r\n");
          if (recvResult <= 0)
          {
            this->errorCode_ = ErrorCode::SocketError;
          }
          else
          {
            sizeLine.resize(recvResult);
            std::string delim = ";";
            auto searchResult = std::search(sizeLine.begin(), sizeLine.end(), delim.begin(), delim.end());
            if (searchResult != sizeLine.end())
            {
              sizeLine = std::string(sizeLine.begin(), searchResult);
            }

            std::stringstream sizeLineStream(sizeLine);
            sizeLineStream >> this->bytesRemainingInChunk_;
          }
        }

        if (this->errorCode_ == ErrorCode::NoError)
        {
          if (bytesRemainingInChunk_ == 0)
          {
            this->eof_ = true;
          }
          else
          {
            ssize_t recvResult = this->socket_.recv(buff, this->bytesRemainingInChunk_ > buffSize ? buffSize : this->bytesRemainingInChunk_);
            if (recvResult <= 0)
            {
              this->errorCode_ = ErrorCode::SocketError;
            }
            else
            {
              this->bytesRemainingInChunk_ -= recvResult;
              ret = recvResult;
            }
          }

          if (bytesRemainingInChunk_ == 0 && this->errorCode_ == ErrorCode::NoError)
          {
            char emptyLineDiscard[2];
            ssize_t recvResult = TCP::recvLine(this->socket_, emptyLineDiscard, 2, "\r\n");
            if (recvResult <= 0)
            {
              this->errorCode_ = ErrorCode::SocketError;
            }
          }
        }
      }



      return ret;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    ssize_t IncomingMessage::recvKnownLengthEntity(char* buff, std::size_t buffSize)
    {
      ssize_t ret = -1;

      if (this->contentLength_ == this->bytesReceived_)
      {
        // TODO: Set error.
      }
      else
      {
        std::size_t bytesRemaining = this->contentLength_ - this->bytesReceived_;

        ret = this->socket_.recv(buff, bytesRemaining < buffSize ? bytesRemaining : buffSize);
        if (ret < 0)
        {
          this->errorCode_ = ErrorCode::SocketError;
        }
        else
        {
          this->bytesReceived_ += ret;

          if (this->bytesReceived_ == this->contentLength_)
            this->eof_ = true;
        }
      }

      return ret;
    }
    //----------------------------------------------------------------//
  }
}