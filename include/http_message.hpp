#pragma once

#ifndef IPSUITE_HTTP_MESSAGE_HPP
#define IPSUITE_HTTP_MESSAGE_HPP

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
      enum class error_code
      { NoError = 0, SocketError, HeadCorrupt, HeadTooLarge };
      //----------------------------------------------------------------//
    protected:
      //----------------------------------------------------------------//
      message_head& head_;
      std::shared_ptr<http::connection> connection_;
      std::int32_t stream_id_;
      transfer_encoding transfer_encoding_;
      std::uint64_t content_length_;
      //----------------------------------------------------------------//
    public:
      //----------------------------------------------------------------//
      message(message_head& head, const std::shared_ptr<http::connection>& conn, std::int32_t stream_id);
      virtual ~message();
      //----------------------------------------------------------------//
    };
    //================================================================//
  }
}

#endif //IPSUITE_HTTP_MESSAGE_HPP
