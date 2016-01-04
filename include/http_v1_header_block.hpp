//
// Created by Jonathon LeFaive on 1/3/16.
//

#ifndef MANIFOLD_HTTP_V1_HEADER_BLOCK_HPP
#define MANIFOLD_HTTP_V1_HEADER_BLOCK_HPP

#include <string>
#include <list>
#include <iostream>

namespace manifold
{
  namespace http
  {
    //================================================================//
    class v1_header_block
    {
    public:
      //----------------------------------------------------------------//
      v1_header_block();
      virtual ~v1_header_block();
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      void header(const std::string& name, const std::string& value);
      void header(std::string&& name, std::string&& value);
      void multi_header(const std::string& name, const std::list<std::string>& values);
      void multi_header(std::string&& name, std::list<std::string>&& values);
      std::string header(const std::string& name) const;
      std::list<std::string> multi_header(const std::string& name) const;
      std::list<std::pair<std::string, std::string>> raw_headers() const;
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
      static void serialize(const v1_header_block& source, std::ostream& destination);
      static bool deserialize(std::istream& source, v1_header_block& destination);
      //----------------------------------------------------------------//
    protected:
      //----------------------------------------------------------------//
      std::string start_line_;
      std::list<std::pair<std::string,std::string>> headers_;
      //----------------------------------------------------------------//
    };
    //================================================================//
  }
}

#endif //MANIFOLD_HTTP_V1_HEADER_BLOCK_HPP
