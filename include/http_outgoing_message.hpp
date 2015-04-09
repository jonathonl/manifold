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
      std::uint64_t bytesSent_;
      bool headersSent_;
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      //bool sendChunkedEntity(const char* data, std::size_t dataSize);
      //bool sendKnownLengthEntity(const char* data, std::size_t dataSize);
      //----------------------------------------------------------------//
    public:
      //----------------------------------------------------------------//
      outgoing_message(message_head& head, const std::shared_ptr<http::connection>& conn, std::int32_t stream_id);
      virtual ~outgoing_message();
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      bool send_head();
      bool send(const char*const data, std::size_t data_sz);
      void on_drain(const std::function<void()>& fn);
      bool end(const char*const data, std::size_t data_sz);
      bool end();
      //----------------------------------------------------------------//
    };
    //================================================================//
  }
}

#endif //MANIFOLD_HTTP_OUTGOING_MESSAGE_HPP
