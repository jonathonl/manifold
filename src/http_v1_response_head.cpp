
#include <sstream>

#include "http_v1_response_head.hpp"

namespace manifold
{
  namespace http
  {
    //----------------------------------------------------------------//
    v1_response_head::v1_response_head()
    {
      this->start_line_ = "http/1.1 200 Ok";
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    v1_response_head::v1_response_head(v1_message_head&& hb)
      : v1_message_head(std::move(hb))
    {
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    v1_response_head::v1_response_head(const v1_message_head& hb)
      : v1_message_head(hb)
    {
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    v1_response_head::v1_response_head(const response_head& generic_head)
    {
      this->start_line_ = "http/1.1 " + std::to_string(generic_head.status_code()) + " " + status_code_to_reason_phrase(generic_head.status_code());
      this->headers_ = generic_head.raw_headers();
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    v1_response_head::v1_response_head(unsigned short status, std::list<std::pair<std::string,std::string>>&& headers)
    {
      this->start_line_ = "http/1.1 200 Ok";
      this->headers_ = std::move(headers);
      this->status_code(status);
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    v1_response_head::v1_response_head(http::status_code status, std::list<std::pair<std::string,std::string>>&& headers)
    {
      this->start_line_ = "http/1.1 200 Ok";
      this->headers_ = std::move(headers);
      this->status_code(status);
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    v1_response_head::~v1_response_head()
    {
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    unsigned short v1_response_head::status_code() const
    {
      unsigned short ret = 0;
      std::stringstream tmp(this->start_line_.substr(9,3));
      tmp >> ret;
      return ret;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void v1_response_head::status_code(unsigned short value)
    {
      std::string status_string = std::to_string(value);
      if (status_string.size() == 3)
        this->start_line_ = (this->start_line_.substr(0, 9) + status_string + " " + status_code_to_reason_phrase(value));
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void v1_response_head::status_code(http::status_code value)
    {
      this->status_code((unsigned short)value);
    }
    //----------------------------------------------------------------//
  }
}