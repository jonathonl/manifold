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
    public:
      //----------------------------------------------------------------//
      incoming_message(http::connection& conn, std::int32_t stream_id);
      incoming_message(incoming_message&& source);
      virtual ~incoming_message();
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      void recv(char* buf, std::size_t sz);
      bool eof();
      //----------------------------------------------------------------//
    private:
      http::header_block trailers_;
    };
    //================================================================//
  }
}

#endif //MANIFOLD_HTTP_INCOMING_MESSAGE_HPP
