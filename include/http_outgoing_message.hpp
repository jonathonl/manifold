#pragma once

#ifndef MANIFOLD_HTTP_OUTGOING_MESSAGE_HPP
#define MANIFOLD_HTTP_OUTGOING_MESSAGE_HPP

#include "http_message.hpp"

namespace manifold
{
  namespace http
  {
    //================================================================//
    class outgoing_message : public message
    {
    private:
      //----------------------------------------------------------------//
      std::uint64_t bytes_sent_;
      bool headers_sent_;
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      //bool sendChunkedEntity(const char* data, std::size_t dataSize);
      //bool sendKnownLengthEntity(const char* data, std::size_t dataSize);
      //----------------------------------------------------------------//
    public:
      //----------------------------------------------------------------//
      outgoing_message(header_block& head, const std::shared_ptr<http::connection>& conn, std::int32_t stream_id);
      virtual ~outgoing_message();
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      void reset_stream(http::errc error_code = http::errc::no_error) { /*TODO: impl*/ }
      bool send_headers(bool end_stream = false);
      bool send(const char*const data, std::size_t data_sz);
      void on_drain(const std::function<void()>& fn);
      bool end(const char*const data, std::size_t data_sz, const http::header_block& trailers = {});
      template <typename BufferT>
      bool end(const BufferT& dataBuffer, const http::header_block& trailers = {})
      {
        return this->end(dataBuffer.data(), dataBuffer.size(), trailers);
      }
      bool end(const http::header_block& trailers = {});
      //----------------------------------------------------------------//
    };
    //================================================================//
  }
}

#endif //MANIFOLD_HTTP_OUTGOING_MESSAGE_HPP
