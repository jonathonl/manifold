#pragma once

#ifndef MANIFOLD_HTTP_MESSAGE_HPP
#define MANIFOLD_HTTP_MESSAGE_HPP

#include "http_connection.hpp"
#include "http_message_head.hpp"
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
//      enum class error_code
//      { NoError = 0, SocketError, HeadCorrupt, HeadTooLarge };
      //----------------------------------------------------------------//
    protected:
      //----------------------------------------------------------------//
      std::shared_ptr<http::connection> connection_;
      std::uint32_t stream_id_;
      bool ended_ = false;
      //----------------------------------------------------------------//
    public:
      //----------------------------------------------------------------//
      message(const std::shared_ptr<http::connection>& conn, std::uint32_t stream_id);
      //message(message&& source) {}
      //message& operator=(message&& source) { return *this; }

      virtual ~message();
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      std::uint32_t stream_id() const;
      void close(http::errc error_code = http::errc::no_error);
      void on_close(const std::function<void(std::uint32_t ec)>& cb);
      //----------------------------------------------------------------//
    private:
      //message(const message&) = delete;
      //message& operator=(const message&) = delete;
    };
    //================================================================//
  }
}

#endif //MANIFOLD_HTTP_MESSAGE_HPP
