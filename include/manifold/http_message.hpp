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
      message(http::connection& conn, std::uint32_t stream_id);
      message(message&& source);
      virtual ~message();
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      std::uint32_t stream_id() const;
      ::manifold::http::version http_version() const;
      http::header_block& header_block();
      void cancel();
      //void on_close(const std::function<void(const std::error_code& ec)>& cb);
      //----------------------------------------------------------------//
      message(const message&) = delete;
      message& operator=(const message&) = delete;
      message& operator=(message&&) = delete;
    protected:
      //----------------------------------------------------------------//
      http::connection& connection_;
      http::header_block headers_;
      std::uint32_t stream_id_;
      //----------------------------------------------------------------//
    };
    //================================================================//
  }
}

#endif //MANIFOLD_HTTP_MESSAGE_HPP
