#pragma once

#ifndef MANIFOLD_HTTP_MESSAGE_HPP
#define MANIFOLD_HTTP_MESSAGE_HPP

#include "http_connection.hpp"
#include "http_header_block.hpp"
#include "hpack.hpp"

namespace manifold
{
  namespace http
  {
    //================================================================//
    class message
    {
    public:
      //----------------------------------------------------------------//
      message(const std::shared_ptr<connection::stream>& stream_ptr);
      message(message&& source);
      virtual ~message();
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      std::uint32_t stream_id() const;
      //::manifold::http::version http_version() const;
      void cancel();
      //void on_close(const std::function<void(const std::error_code& ec)>& cb);
      //----------------------------------------------------------------//
      message(const message&) = delete;
      message& operator=(const message&) = delete;
      message& operator=(message&&) = delete;
    protected:
      virtual http::header_block& message_head() = 0;
    protected:
      //----------------------------------------------------------------//
      std::shared_ptr<connection::stream> stream_;
      //http::header_block headers_;
      //----------------------------------------------------------------//
    };
    //================================================================//
  }
}

#endif //MANIFOLD_HTTP_MESSAGE_HPP
