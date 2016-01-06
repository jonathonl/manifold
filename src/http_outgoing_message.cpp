
#include <sstream>

#include "http_outgoing_message.hpp"
#include "tcp.hpp"

namespace manifold
{
  namespace http
  {
    //----------------------------------------------------------------//
    outgoing_message::outgoing_message(const std::shared_ptr<http::connection>& conn, std::int32_t stream_id)
      : message(conn, stream_id),
        bytes_sent_(0),
        headers_sent_(false)
    {
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    outgoing_message::~outgoing_message()
    {
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    bool outgoing_message::send_headers(bool end_stream)
    {
      bool ret = false;

      if (!this->headers_sent_)
      {
        ret = this->connection_->send_headers(this->stream_id_, this->message_head(), true, end_stream);
        this->headers_sent_= true;
        this->ended_ = end_stream;
      }

      return ret;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    bool outgoing_message::send(const char*const data, std::size_t data_sz)
    {
      bool ret = true;
      if (!this->headers_sent_)
        ret =this->send_headers();

      if (ret)
      {
        ret = this->connection_->send_data(this->stream_id_, data, data_sz, false);
      }

      return ret;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void outgoing_message::on_drain(const std::function<void()>& fn)
    {
      this->connection_->on_drain(this->stream_id_, fn);
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    bool outgoing_message::end(const char*const data, std::size_t data_sz, const v2_header_block& trailers)
    {
      bool ret = true;
      if (!this->ended_)
      {
        if (!this->headers_sent_)
          ret = this->send_headers();

        if (ret)
        {
          this->connection_->send_data(this->stream_id_, data, data_sz, trailers.empty());
          // TODO: Check content length against amount sent;
          if (!trailers.empty())
            this->connection_->send_headers(this->stream_id_, trailers, true, true);
          this->ended_ = true;
        }
      }

      return ret;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    bool outgoing_message::end(const v2_header_block& trailers)
    {
      return this->end(nullptr, 0, trailers);
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