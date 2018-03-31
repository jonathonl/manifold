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
      incoming_message(const std::shared_ptr<connection::stream>& stream_ptr);
      incoming_message(incoming_message&& source);
      virtual ~incoming_message();
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      future<std::size_t> recv(char* buf, std::size_t sz);
      operator bool() const;
      bool eof();
      //----------------------------------------------------------------//
    private:
      http::header_block trailers_;
    };
    //================================================================//
  }
}

#endif //MANIFOLD_HTTP_INCOMING_MESSAGE_HPP
