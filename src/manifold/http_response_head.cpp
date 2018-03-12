
#include <sstream>

#include "manifold/http_response_head.hpp"

namespace manifold
{
  namespace http
  {
    //----------------------------------------------------------------//
    response_head::response_head()
    {
      this->set_status_code(status_code::ok);
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    response_head::response_head(const response_head& generic_head)
      : header_block(generic_head)
    {
      this->set_status_code(generic_head.get_status_code());
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    response_head::response_head(header_block&& hb)
      : header_block(std::move(hb))
    {
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    response_head::response_head(unsigned short status, std::list<hpack::header_field>&& headers)
    {
      this->headers_ = std::move(headers);
      this->set_status_code(status);
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    response_head::response_head(http::status_code status, std::list<hpack::header_field>&& headers)
    {
      this->headers_ = std::move(headers);
      this->set_status_code(status);
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    response_head::~response_head()
    {
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    unsigned short response_head::get_status_code() const
    {
      std::stringstream tmp(this->header(":status"));
      unsigned short ret = 0;
      tmp >> ret;
      return ret;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void response_head::set_status_code(unsigned short value)
    {
      this->pseudo_header(":status", std::to_string(value));
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void response_head::set_status_code(http::status_code value)
    {
      this->pseudo_header(":status", std::to_string((unsigned short)value));
    }
    //----------------------------------------------------------------//
  }
}