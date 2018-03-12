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
      outgoing_message(http::connection& conn, std::int32_t stream_id);
      outgoing_message(outgoing_message&& source);
      virtual ~outgoing_message();
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      virtual bool send_headers(bool end_stream = false); // Must be virtual since client::request and server::response override while outgoing_message::end/send call this method.
      bool send(const char* data, std::size_t data_sz);
      bool send(const char* cstr) { return this->send(std::string(cstr)); }
      template <typename BufferT>
      bool send(const BufferT& dataBuffer)
      {
        return this->send(dataBuffer.data(), dataBuffer.size());
      }
      void on_drain(const std::function<void()>& fn);


      bool end(const char* data, std::size_t data_sz);
      bool end(const char* cstr)
      {
        return this->end(std::string(cstr));
      }
      template <typename BufferT>
      bool end(const BufferT& dataBuffer)
      {
        return this->end(dataBuffer.data(), dataBuffer.size());
      }
      bool end();

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
