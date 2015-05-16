#pragma once

#ifndef MANIFOLD_HTTP_MESSAGE_HEAD_HPP
#define MANIFOLD_HTTP_MESSAGE_HEAD_HPP

#include <string>
#include <list>

#include "socket.hpp"
#include "hpack.hpp"
#include "http_frame.hpp"

namespace manifold
{
  namespace http
  {
    // Reference URL: http://tools.ietf.org/html/rfc2616#section-4.4

    //================================================================//
    class header_block
    {
    protected:
      //----------------------------------------------------------------//
      std::list<hpack::header_field> headers_;
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      void pseudo_header(const std::string& name, const std::string& value);
      void pseudo_header(std::string&& name, std::string&& value);
      //----------------------------------------------------------------//
    public:
      //----------------------------------------------------------------//
      header_block();
      header_block(std::list<hpack::header_field>&& raw_headers);
      virtual ~header_block();
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      void header(const std::string& name, const std::string& value);
      void header(std::string&& name, std::string&& value);
      void multi_header(const std::string& name, const std::list<std::string>& values);
      void multi_header(std::string&& name, std::list<std::string>&& values);
      std::string header(const std::string& name) const;
      std::list<std::string> multi_header(const std::string& name) const;
      const std::list<hpack::header_field>& raw_headers() const;
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      bool empty() const { return this->headers_.empty(); }
      std::size_t size() const { return this->headers_.size(); }
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
//      const std::string& http_version() const;
//      void http_version(const std::string& version); // TODO: Make this an enum.
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
//      virtual void start_line(const std::string& value) = 0;
//      virtual std::string start_line() const = 0;
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      static void serialize(hpack::encoder& enc, const header_block& source, std::string& destination);
      static bool deserialize(hpack::decoder& dec, const std::string& source, header_block& destination);
      //----------------------------------------------------------------//
    };
    //================================================================//
  }
}

#endif // MANIFOLD_HTTP_MESSAGE_HEAD_HPP