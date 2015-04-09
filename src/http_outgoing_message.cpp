
#include <sstream>

#include "http_outgoing_message.hpp"
#include "tcp.hpp"

namespace manifold
{
  namespace http
  {
    //----------------------------------------------------------------//
    outgoing_message::outgoing_message(message_head& head, const std::shared_ptr<http::connection>& conn, std::int32_t stream_id)
      : message(head, conn, stream_id),
        bytesSent_(0),
        headersSent_(false)
    {
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    outgoing_message::~outgoing_message()
    {
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    bool outgoing_message::send_head()
    {
      bool ret = false;

      if (!this->headersSent_)
      {
        ret = this->connection_->send_headers(this->stream_id_, this->head_);
      }

      return ret;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    bool outgoing_message::send(const char*const data, std::size_t data_sz)
    {
      bool ret = true;
      if (!this->headersSent_)
        ret = this->send_head();

      if (ret)
      {
        ret = this->connection_->send_data(this->stream_id_, data, data_sz);
      }

      return ret;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void outgoing_message::on_drain(const std::function<void()>& fn)
    {
      this->connection_->on_window_update(this->stream_id_, fn);
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    bool outgoing_message::end(const char*const data, std::size_t data_sz)
    {
      bool ret = true;
      if (!this->headersSent_)
        ret = this->send_head();

      if (ret)
      {
        this->connection_->send_end_frame(this->stream_id_, data, data_sz);
        // TODO: Check content length against amount sent;
      }

      return ret;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    bool outgoing_message::end()
    {
      this->connection_->send_end_frame(this->stream_id_);

      return true; // TODO: Check content length against amount sent;
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
//      http::message_head::serialize(this->head_, headerString);
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