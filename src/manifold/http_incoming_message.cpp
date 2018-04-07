
#include "manifold/http_incoming_message.hpp"

namespace manifold
{
  namespace http
  {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
    incoming_message::incoming_message(const std::shared_ptr<connection::stream>& stream_ptr)
      : message(stream_ptr)
    {

    }
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
    incoming_message::incoming_message(incoming_message&& source)
      : message(std::move(source)), trailers_(std::move(source.trailers_))
    {

    }
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
    incoming_message::~incoming_message()
    {

    }
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
    future<std::size_t> incoming_message::recv(char* buf, std::size_t sz)
    {
      return stream_->recv_data(buf, sz);
    }
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
    
    
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
    incoming_message::operator bool() const
    {
      if (stream_->state() == connection::stream_state::closed || stream_->state() == connection::stream_state::half_closed_remote)
      {
        if (!stream_->has_receivable_data())
          return false;
      }
      return true;
    }
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
    bool incoming_message::eof()
    {
      return false;
      // TODO:
    }
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

//    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//    ssize_t incoming_message::recv(char* buff, std::size_t buffSize)
//    {
//      ssize_t ret = -1;
//      if (this->transfer_encoding_ == transfer_encoding::Chunked)
//        ret = this->recvChunkedEntity(buff, buffSize);
//      else
//        ret = this->recvKnownLengthEntity(buff, buffSize);
//      return ret;
//    }
//    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//
//    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//    ssize_t incoming_message::recvChunkedEntity(char* buff, std::size_t buffSize)
//    {
//      ssize_t ret = -1;
//
//      if (!this->eof_)
//      {
//        if (bytesRemainingInChunk_ == 0)
//        {
//          std::string sizeLine(2048, '\0');
//          ssize_t recvResult = 0; //TCP::recvline(this->socket_, &sizeLine[0], 2048, "\r\n", nullptr);
//          if (recvResult <= 0)
//          {
//            this->error_code_ = error_code::SocketError;
//          }
//          else
//          {
//            sizeLine.resize(recvResult);
//            std::string delim = ";";
//            auto searchResult = std::search(sizeLine.begin(), sizeLine.end(), delim.begin(), delim.end());
//            if (searchResult != sizeLine.end())
//            {
//              sizeLine = std::string(sizeLine.begin(), searchResult);
//            }
//
//            std::stringstream sizeLineStream(sizeLine);
//            sizeLineStream >> this->bytesRemainingInChunk_;
//          }
//        }
//
//        if (this->error_code_ == error_code::NoError)
//        {
//          if (bytesRemainingInChunk_ == 0)
//          {
//            this->eof_ = true;
//          }
//          else
//          {
//            ssize_t recvResult = this->socket_.recv(buff, this->bytesRemainingInChunk_ > buffSize ? buffSize : this->bytesRemainingInChunk_);
//            if (recvResult <= 0)
//            {
//              this->error_code_ = error_code::SocketError;
//            }
//            else
//            {
//              this->bytesRemainingInChunk_ -= recvResult;
//              ret = recvResult;
//            }
//          }
//
//          if (bytesRemainingInChunk_ == 0 && this->error_code_ == error_code::NoError)
//          {
//            char emptyLineDiscard[2];
//            ssize_t recvResult = 0; //TCP::recvline(this->socket_, emptyLineDiscard, 2, "\r\n");
//            if (recvResult <= 0)
//            {
//              this->error_code_ = error_code::SocketError;
//            }
//          }
//        }
//      }
//
//
//
//      return ret;
//    }
//    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//
//    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//    ssize_t incoming_message::recvKnownLengthEntity(char* buff, std::size_t buffSize)
//    {
//      ssize_t ret = -1;
//
//      if (this->content_length_ == this->bytesReceived_)
//      {
//        // TODO: Set error.
//      }
//      else
//      {
//        std::size_t bytesRemaining = this->content_length_ - this->bytesReceived_;
//
//        ret = this->socket_.recv(buff, bytesRemaining < buffSize ? bytesRemaining : buffSize);
//        if (ret < 0)
//        {
//          this->error_code_ = error_code::SocketError;
//        }
//        else
//        {
//          this->bytesReceived_ += ret;
//
//          if (this->bytesReceived_ == this->content_length_)
//            this->eof_ = true;
//        }
//      }
//
//      return ret;
//    }
//    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
  }
}