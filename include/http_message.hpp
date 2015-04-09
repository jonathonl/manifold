#pragma once

#ifndef MANIFOLD_HTTP_MESSAGE_HPP
#define MANIFOLD_HTTP_MESSAGE_HPP

#include "http_connection.hpp"
#include "http_message_head.hpp"

namespace manifold
{
  namespace http
  {
    //================================================================//
    class message
    {
    public:
      //----------------------------------------------------------------//
//      enum class error_code
//      { NoError = 0, SocketError, HeadCorrupt, HeadTooLarge };
      //----------------------------------------------------------------//
    protected:
      //----------------------------------------------------------------//
      message_head& head_;
      std::shared_ptr<http::connection> connection_;
      std::uint32_t stream_id_;
      transfer_encoding transfer_encoding_;
      std::uint64_t content_length_;
      //----------------------------------------------------------------//
    public:
      //----------------------------------------------------------------//
      message(message_head& head, const std::shared_ptr<http::connection>& conn, std::uint32_t stream_id);
      virtual ~message();
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      std::uint32_t stream_id() const;
      void on_stream_reset(const std::function<void(const std::error_code& ec)>& cb);
      //----------------------------------------------------------------//
    };
    //================================================================//
  }
}

#endif //MANIFOLD_HTTP_MESSAGE_HPP
