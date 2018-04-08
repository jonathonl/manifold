// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

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
    public:
      //----------------------------------------------------------------//
      outgoing_message(const std::shared_ptr<connection::stream>& stream_ptr);
      outgoing_message(outgoing_message&& source);
      virtual ~outgoing_message();
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      virtual future<bool> send_headers(bool end_stream = false); // Must be virtual since client::request and server::response override while outgoing_message::end/send call this method.
      future<std::size_t> send(const char* data, std::size_t data_sz);
      future<std::size_t> send(const char* cstr) { return this->send(cstr, std::strlen(cstr)); }
      template <typename BufferT>
      future<std::size_t> send(const BufferT& dataBuffer)
      {
        return this->send(dataBuffer.data(), dataBuffer.size());
      }
      void on_drain(const std::function<void()>& fn);

      operator bool() const;


      future<void> end(const char* data, std::size_t data_sz);
      future<void> end(const char* cstr)
      {
        return this->end(std::string(cstr));
      }
      template <typename BufferT>
      future<void> end(const BufferT& dataBuffer)
      {
        return this->end(dataBuffer.data(), dataBuffer.size());
      }
      future<void> end();

      //----------------------------------------------------------------//
    private:
      //----------------------------------------------------------------//
      bool headers_sent_;
      bool ended_;
      //----------------------------------------------------------------//
    protected:
      //----------------------------------------------------------------//
      //virtual SendMsg& message_head() = 0;
      //----------------------------------------------------------------//
    };
    //================================================================//
  }
}

#endif //MANIFOLD_HTTP_OUTGOING_MESSAGE_HPP
