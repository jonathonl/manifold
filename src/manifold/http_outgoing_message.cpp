
#include "manifold/http_outgoing_message.hpp"

namespace manifold
{
  namespace http
  {
    //----------------------------------------------------------------//
    outgoing_message::outgoing_message(const std::shared_ptr<connection::stream>& stream_ptr)
      : message(stream_ptr),
      headers_sent_(false),
      ended_(false)
    {
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    outgoing_message::outgoing_message(outgoing_message&& source)
      : message(std::move(source)),
      headers_sent_(source.headers_sent_),
      ended_(source.ended_)
    {
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    outgoing_message::~outgoing_message()
    {
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    connection::stream::send_headers_awaiter outgoing_message::send_headers(bool end_stream)
    {
      if (!this->headers_sent_)
      {
        this->headers_sent_= true;
        this->ended_ = end_stream;
        return this->stream_->send_headers(this->header_block(), end_stream);
      }

      return connection::stream::send_headers_awaiter{nullptr};
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    connection::stream::send_data_awaiter outgoing_message::send(const char* data, std::size_t data_sz)
    {
      if (!this->headers_sent_)
        this->send_headers();

      if (!this->ended_)
      {
        return this->stream_->send_data(data, data_sz, false);
      }

      return connection::stream::send_data_awaiter{nullptr, 0};
    }
    //----------------------------------------------------------------//

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
    outgoing_message::operator bool() const
    {
      if (stream_->state() == connection::stream_state::closed || stream_->state() == connection::stream_state::half_closed_local)
      {
        return false;
      }
      return true;
    }
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

    //----------------------------------------------------------------//
    connection::stream::send_data_awaiter outgoing_message::end(const char* data, std::size_t data_sz)
    {
      if (!this->headers_sent_)
        this->send_headers();

      if (!this->ended_)
      {
        // TODO: Check content length against amount sent;
        this->ended_ = true;

        return this->stream_->send_data(data, data_sz, true);
      }

      return connection::stream::send_data_awaiter{nullptr, 0};
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    connection::stream::send_data_awaiter outgoing_message::end()
    {
      return this->end(nullptr, 0);
    }
    //----------------------------------------------------------------//



//    //----------------------------------------------------------------//
//    bool outgoing_message::send(const char* data, std::size_t dataSize)
//    {
//      bool ret = true;
//      if (!this->headersSent_)
//      {
//
//        std::string transferEncoding = this->head_.header("transfer-encoding");
//        std::string contentLengthString = this->head_.header("content-length");
//
//        if (contentLengthString.empty() && transferEncoding.empty())
//        {
//          this->transfer_encoding_ = transfer_encoding::Chunked;
//          this->head_.header("transfer-encoding", "chunked");
//        }
//        else if (contentLengthString.empty() && transferEncoding != "identity")
//        {
//          this->transfer_encoding_ = transfer_encoding::Chunked;
//        }
//        else
//        {
//          this->transfer_encoding_ = transfer_encoding::Identity;
//
//
//          if (contentLengthString.empty())
//          {
//            // multipart/byteranges not supported
//            this->head_.header("content-length", std::to_string(dataSize));
//            this->content_length_ = dataSize;
//          }
//          else
//          {
//            std::stringstream contentLengthStream(contentLengthString);
//            contentLengthStream >> this->content_length_;
//          }
//        }
//
//        ret = this->sendHead();
//
//        this->headersSent_ = true;
//      }
//
//      if (ret)
//      {
//        if (transfer_encoding_ == transfer_encoding::Chunked)
//          ret = this->sendChunkedEntity(data, dataSize);
//        else
//          ret = this->sendKnownLengthEntity(data, dataSize);
//      }
//
//      return ret;
//    }
//    //----------------------------------------------------------------//
//
//    //----------------------------------------------------------------//
//    bool outgoing_message::sendChunkedEntity(const char* data, std::size_t dataSize)
//    {
//      bool ret = false;
//
//      std::stringstream sizeLineStream;
//      sizeLineStream << std::hex << dataSize;
//      sizeLineStream << "\r\n";
//
//      std::string sizeLine(sizeLineStream.str());
//
//
//      if (!TCP::sendAll(this->socket_, sizeLine.data(), sizeLine.size()))
//      {
//        this->error_code_ = error_code::SocketError;
//      }
//      else
//      {
//        if (!TCP::sendAll(this->socket_, sizeLine.data(), sizeLine.size()))
//        {
//          this->error_code_ = error_code::SocketError;
//        }
//        else
//        {
//          this->bytesSent_ += dataSize;
//
//          const char* newLine = "\r\n";
//          if (!TCP::sendAll(this->socket_, newLine, 2))
//          {
//            this->error_code_ = error_code::SocketError;
//          }
//          else
//          {
//            ret = true;
//          }
//        }
//      }
//
//      return ret;
//    }
//    //----------------------------------------------------------------//
//
//    //----------------------------------------------------------------//
//    bool outgoing_message::sendKnownLengthEntity(const char* data, std::size_t dataSize)
//    {
//      bool ret = false;
//
//      std::size_t bytesRemaining = (this->content_length_ - this->bytesSent_);
//      if (bytesRemaining == 0)
//      {
//        // TODO: Set error.
//      }
//      else if (bytesRemaining < dataSize)
//      {
//        // TODO: Set error.
//      }
//      else
//      {
//        if (!TCP::sendAll(this->socket_, data, dataSize))
//        {
//          this->error_code_ = error_code::SocketError;
//        }
//        else
//        {
//          this->bytesSent_ += dataSize;
//
//          if (this->bytesSent_ == this->content_length_)
//            this->eof_ = true;
//
//          ret = true;
//        }
//      }
//
//      return ret;
//    }
//    //----------------------------------------------------------------//
//
//    //----------------------------------------------------------------//
//    bool outgoing_message::sendHead()
//    {
//      bool ret = false;
//
//      std::string headerString;
//      http::header_block::serialize(this->head_, headerString);
//
//      if (!TCP::sendAll(this->socket_, headerString.data(), headerString.size()))
//      {
//        this->error_code_ = error_code::SocketError;
//      }
//      else
//      {
//        ret = true;
//      }
//
//      return ret;
//    }
//    //----------------------------------------------------------------//
  }
}