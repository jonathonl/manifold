
#include <sstream>

#include "http_response_head.hpp"

namespace IPSuite
{
  namespace HTTP
  {
    //----------------------------------------------------------------//
    ResponseHead::ResponseHead()
    {
      this->statusCode_ = (unsigned short)StatusCode::Ok;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    ResponseHead::~ResponseHead()
    {
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    unsigned short ResponseHead::statusCode() const
    {
      return this->statusCode_;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void ResponseHead::statusCode(unsigned short value)
    {
      this->statusCode_ = value;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void ResponseHead::statusCode(StatusCode value)
    {
      this->statusCode_ = (unsigned short)value;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    const std::string& ResponseHead::reasonPhrase() const
    {
      return this->reasonPhrase_;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void ResponseHead::reasonPhrase(const std::string& value)
    {
      this->reasonPhrase_ = value;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void ResponseHead::startLine(const std::string& value)
    {
      std::stringstream ss(value);
      std::getline(ss, this->version_, ' ');
      this->statusCode_ = 0;
      ss >> this->statusCode_;
      std::getline(ss, this->reasonPhrase_);
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    std::string ResponseHead::startLine() const
    {
      return this->version_ + " " + std::to_string(this->statusCode_) + " " + this->reasonPhrase_;
    }
    //----------------------------------------------------------------//
  }
}