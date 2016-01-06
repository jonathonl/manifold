#pragma once

#ifndef MANIFOLD_HTTP_INCOMING_MESSAGE_HPP
#define MANIFOLD_HTTP_INCOMING_MESSAGE_HPP

#include "socket.hpp"
#include "http_message.hpp"

namespace manifold
{
  namespace http
  {
    //================================================================//
    class incoming_message : public message
    {
    private:
      //----------------------------------------------------------------//
      std::uint64_t bytesReceived_;
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      std::uint64_t bytesRemainingInChunk_;
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      //ssize_t recvChunkedEntity(char* buff, std::size_t buffSize);
      //ssize_t recvKnownLengthEntity(char* buff, std::size_t buffSize);
      //----------------------------------------------------------------//
    protected:
      //----------------------------------------------------------------//
      virtual v2_header_block& message_head() = 0;
      //----------------------------------------------------------------//
    public:
      //----------------------------------------------------------------//
      //incoming_message(const header_block& head, Socket&& sock);
      incoming_message(const std::shared_ptr<http::connection>& conn, std::int32_t stream_id);
      virtual ~incoming_message();
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      void on_data(const std::function<void(const char*const buff, std::size_t buff_size)>& fn);
      void on_end(const std::function<void()>& fn);
      //----------------------------------------------------------------//
    };
    //================================================================//
  }
}

#endif //MANIFOLD_HTTP_INCOMING_MESSAGE_HPP
