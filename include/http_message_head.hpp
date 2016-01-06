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
      header_block();
      virtual ~header_block();
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      void header(const std::string& name, const std::string& value);
      void header(std::string&& name, std::string&& value);
      void multi_header(const std::string& name, const std::list<std::string>& values);
      void multi_header(std::string&& name, std::list<std::string>&& values);
      std::string header(const std::string& name) const;
      std::list<std::string> multi_header(const std::string& name) const;
      const std::list<std::pair<std::string,std::string>>& raw_headers() const;
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      bool empty() const;
      std::size_t size() const;
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
//      const std::string& http_version() const;
//      void http_version(const std::string& version); // TODO: Make this an enum.
      //----------------------------------------------------------------//
    protected:
      //----------------------------------------------------------------//
      std::list<std::pair<std::string,std::string>> headers_;
      //----------------------------------------------------------------//
    };
    //================================================================//
  }
}

#endif // MANIFOLD_HTTP_MESSAGE_HEAD_HPP