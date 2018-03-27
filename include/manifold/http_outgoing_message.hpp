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
      virtual connection::stream::send_headers_awaiter send_headers(bool end_stream = false); // Must be virtual since client::request and server::response override while outgoing_message::end/send call this method.
      connection::stream::send_data_awaiter send(const char* data, std::size_t data_sz);
      connection::stream::send_data_awaiter send(const char* cstr) { return this->send(std::string(cstr)); }
      template <typename BufferT>
      connection::stream::send_data_awaiter send(const BufferT& dataBuffer)
      {
        return this->send(dataBuffer.data(), dataBuffer.size());
      }
      void on_drain(const std::function<void()>& fn);

      operator bool() const;


      connection::stream::send_data_awaiter end(const char* data, std::size_t data_sz);
      connection::stream::send_data_awaiter end(const char* cstr)
      {
        return this->end(std::string(cstr));
      }
      template <typename BufferT>
      connection::stream::send_data_awaiter end(const BufferT& dataBuffer)
      {
        return this->end(dataBuffer.data(), dataBuffer.size());
      }
      connection::stream::send_data_awaiter end();

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
