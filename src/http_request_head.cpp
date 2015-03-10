
#include <sstream>
#include <algorithm>

#include "http_request_head.hpp"

namespace IPSuite
{
  namespace HTTP
  {
    std::string methodEnumToString(Method method)
    {
      std::string ret;

      if (method == Method::Head) ret         = "HEAD";
      else if (method == Method::Get) ret     = "GET";
      else if (method == Method::Post) ret    = "POST";
      else if (method == Method::Put) ret     = "PUT";
      else if (method == Method::Delete) ret  = "DELETE";
      else if (method == Method::Options) ret = "OPTIONS";
      else if (method == Method::Trace) ret   = "TRACE";
      else if (method == Method::Connect) ret = "CONNECT";
      else if (method == Method::Patch) ret   = "PATCH";

      return ret;
    }

    //----------------------------------------------------------------//
    RequestHead::RequestHead()
    {
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    RequestHead::~RequestHead()
    {
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    const std::string& RequestHead::method() const
    {
      return this->method_;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void RequestHead::method(const std::string& value)
    {
      this->method_ = value;
      std::for_each(this->method_.begin(), this->method_.end(), ::toupper);
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void RequestHead::method(Method value)
    {
      this->method_ = methodEnumToString(value);
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    bool RequestHead::methodIs(Method methodToCheck) const
    {
      return (this->method_ == methodEnumToString(methodToCheck));
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    const std::string& RequestHead::url() const
    {
      return this->url_;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void RequestHead::url(const std::string& value)
    {
      this->url_ = value;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void RequestHead::startLine(const std::string& value)
    {
      std::stringstream ss(value);
      std::getline(ss, this->method_, ' ');
      std::getline(ss, this->url_, ' ');
      std::getline(ss, this->version_);
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    std::string RequestHead::startLine() const
    {
      return this->method_ + " " + this->url_ + " " + this->version_;
    }
    //----------------------------------------------------------------//
  }
}