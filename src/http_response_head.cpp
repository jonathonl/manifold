
#include <sstream>

#include "http_response_head.hpp"

namespace manifold
{
  namespace http
  {
    //----------------------------------------------------------------//
    response_head::response_head()
    {
      this->status_code(status_code::ok);
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    response_head::response_head(header_block&& hb)
      : header_block(hb)
    {
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    response_head::response_head(unsigned short status, std::list<hpack::header_field>&& headers)
    {
      this->headers_ = std::move(headers);
      this->status_code(status);
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    response_head::response_head(http::status_code status, std::list<hpack::header_field>&& headers)
    {
      this->headers_ = std::move(headers);
      this->status_code(status);
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    response_head::~response_head()
    {
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    unsigned short response_head::status_code() const
    {
      std::stringstream tmp(this->header(":status"));
      unsigned short ret = 0;
      tmp >> ret;
      return ret;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void response_head::status_code(unsigned short value)
    {
      this->pseudo_header(":status", std::to_string(value));
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void response_head::status_code(http::status_code value)
    {
      this->pseudo_header(":status", std::to_string((unsigned short)value));
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    bool response_head::is_informational_status() const
    {
      unsigned short status = this->status_code();
      return (status >= 100 && status < 200);
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    bool response_head::is_successful_status() const
    {
      unsigned short status = this->status_code();
      return (status >= 200 && status < 300);
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    bool response_head::is_redirection_status() const
    {
      unsigned short status = this->status_code();
      return (status >= 300 && status < 400);
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    bool response_head::is_client_error_status() const
    {
      unsigned short status = this->status_code();
      return (status >= 400 && status < 500);
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    bool response_head::is_server_error_status() const
    {
      unsigned short status = this->status_code();
      return (status >= 500 && status < 600);
    }
    //----------------------------------------------------------------//
  }
}