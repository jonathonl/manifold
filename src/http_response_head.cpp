
#include <sstream>

#include "http_response_head.hpp"

namespace manifold
{
  namespace http
  {
    //----------------------------------------------------------------//
    response_head::response_head()
    {
      this->status_code_ = (unsigned short) status_code::ok;
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
      return this->status_code_;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void response_head::status_code(unsigned short value)
    {
      this->status_code_ = value;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void response_head::status_code(http::status_code value)
    {
      this->status_code_ = (unsigned short)value;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    const std::string& response_head::reason_phrase() const
    {
      return this->reason_phrase_;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void response_head::reason_phrase(const std::string& value)
    {
      this->reason_phrase_ = value;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void response_head::start_line(const std::string& value)
    {
      std::stringstream ss(value);
      std::getline(ss, this->version_, ' ');
      this->status_code_ = 0;
      ss >> this->status_code_;
      std::getline(ss, this->reason_phrase_);
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    std::string response_head::start_line() const
    {
      return this->version_ + " " + std::to_string(this->status_code_) + " " + this->reason_phrase_;
    }
    //----------------------------------------------------------------//
  }
}