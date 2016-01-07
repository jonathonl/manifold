//
// Created by Jonathon LeFaive on 1/5/16.
//

#ifndef MANIFOLD_HTTP_V1_RESPONSE_HEAD_HPP
#define MANIFOLD_HTTP_V1_RESPONSE_HEAD_HPP

#include "http_v1_message_head.hpp"
#include "http_response_head.hpp"

namespace manifold
{
  namespace http
  {
    //================================================================//
    class v1_response_head : public v1_message_head
    {
    private:
    public:
      //----------------------------------------------------------------//
      v1_response_head();
      v1_response_head(v1_message_head&& hb);
      v1_response_head(const v1_message_head& hb);
      v1_response_head(unsigned short status, std::list<std::pair<std::string,std::string>>&& headers = {});
      v1_response_head(http::status_code status, std::list<std::pair<std::string,std::string>>&& headers = {});
      ~v1_response_head();
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      unsigned short status_code() const;
      void status_code(unsigned short value);
      void status_code(http::status_code value);
      //----------------------------------------------------------------//
    };
    //================================================================//
  };
}

#endif //MANIFOLD_HTTP_V1_RESPONSE_HEAD_HPP
