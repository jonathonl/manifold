
#include <sstream>
#include <algorithm>

#include "http_v1_request_head.hpp"

namespace manifold
{
  namespace http
  {
    //----------------------------------------------------------------//
    v1_request_head::v1_request_head(v1_message_head&& hb)
      : v1_message_head(std::move(hb))
    {
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    v1_request_head::v1_request_head(const request_head& generic_head)
    {
      this->start_line_ = generic_head.method() + " " + generic_head.path() + " http/1.1";
      this->headers_ = generic_head.raw_headers();
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    v1_request_head::v1_request_head(const std::string& path, const std::string& method, std::list<std::pair<std::string,std::string>>&& headers)
    {
      this->start_line_ = "GET / http/1.1";
      this->headers_ = std::move(headers);
      this->path(path);
      this->method(method);
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    v1_request_head::v1_request_head(const std::string& path, http::method method, std::list<std::pair<std::string,std::string>>&& headers)
    {
      this->start_line_ = "GET / http/1.1";
      this->headers_ = std::move(headers);
      this->path(path);
      this->method(method);
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    v1_request_head::~v1_request_head()
    {
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    std::string v1_request_head::method() const
    {
      std::size_t first_space = this->start_line_.find(' ');
      return this->start_line_.substr(0, first_space);
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void v1_request_head::method(const std::string& value)
    {
      std::string tmp(value);
      std::for_each(tmp.begin(), tmp.end(), ::toupper);

      std::size_t first_space = this->start_line_.find(' ');
      this->start_line_ = std::move(tmp) + this->start_line_.substr(first_space);
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void v1_request_head::method(http::method value)
    {
      this->method(method_enum_to_string(value));
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    std::string v1_request_head::path() const
    {
      std::size_t first_space = this->start_line_.find(' ');
      std::size_t last_space = this->start_line_.rfind(' ');
      ++first_space;
      return this->start_line_.substr(first_space, last_space - first_space);
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void v1_request_head::path(const std::string& value)
    {
      std::size_t first_space = this->start_line_.find(' ');
      std::size_t last_space = this->start_line_.rfind(' ');
      ++first_space;
      this->start_line_.replace(first_space, last_space - first_space, value);
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    std::string v1_request_head::scheme() const
    {
      return "";
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void v1_request_head::scheme(const std::string& value)
    {

    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    std::string v1_request_head::authority() const
    {
      return this->header("host");
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void v1_request_head::authority(const std::string& value)
    {
      this->header("host", value);
    }
    //----------------------------------------------------------------//
  }
}