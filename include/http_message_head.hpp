#pragma once

#ifndef MANIFOLD_HTTP_MESSAGE_HEAD_HPP
#define MANIFOLD_HTTP_MESSAGE_HEAD_HPP

#include <string>
#include <list>

namespace manifold
{
  namespace http
  {
    //================================================================//
    class header_block
    {
    public:
      //----------------------------------------------------------------//
      virtual void header(const std::string& name, const std::string& value) = 0;
      virtual void header(std::string&& name, std::string&& value) = 0;
      virtual void multi_header(const std::string& name, const std::list<std::string>& values) = 0;
      virtual void multi_header(std::string&& name, std::list<std::string>&& values) = 0;
      virtual std::string header(const std::string& name) const = 0;
      virtual std::list<std::string> multi_header(const std::string& name) const = 0;
      virtual const std::list<std::pair<std::string,std::string>>& raw_headers() const = 0;
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      virtual bool empty() const = 0;
      virtual std::size_t size() const = 0;
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
//      const std::string& http_version() const;
//      void http_version(const std::string& version); // TODO: Make this an enum.
      //----------------------------------------------------------------//
    };
    //================================================================//
  }
}

#endif // MANIFOLD_HTTP_MESSAGE_HEAD_HPP