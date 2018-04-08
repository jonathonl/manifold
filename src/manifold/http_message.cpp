// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "manifold/http_message.hpp"

namespace manifold
{
  namespace http
  {
    //----------------------------------------------------------------//
    message::message(const std::shared_ptr<connection::stream>& stream_ptr) :
      stream_(stream_ptr)
    {
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    message::message(message&& source)
      : stream_(source.stream_)
    {
      source.stream_ = nullptr;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    message::~message()
    {
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    std::uint32_t message::stream_id() const
    {
      return this->stream_->id();
    }
    //----------------------------------------------------------------//

//    //----------------------------------------------------------------//
//    ::manifold::http::version message::http_version() const
//    {
//      return this->http_version_;
//    }
//    //----------------------------------------------------------------//

//    //----------------------------------------------------------------//
//    http::header_block& message::header_block()
//    {
//      auto s = headers_.size();
//      return headers_;
//    }
//    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void message::cancel()
    {
      this->stream_->send_reset(v2_errc::cancel);
    }
    //----------------------------------------------------------------//

//    //----------------------------------------------------------------//
//    void message::on_close(const std::function<void(const std::error_code&)>& cb)
//    {
//      if (this->connection_)
//        this->connection_->on_close(this->stream_id_, cb);
//    }
//    //----------------------------------------------------------------//

//    //----------------------------------------------------------------//
//    std::string message::errorMessage() const
//    {
//      switch (this->error_code_)
//      {
//        case error_code::SocketError: return "Socket Error";
//        case error_code::HeadCorrupt: return "Headers Are Corrupt";
//        case error_code::HeadTooLarge: return "Headers Are Too Large";
//        default: return "";
//      }
//    }
//    //----------------------------------------------------------------//
  }
}