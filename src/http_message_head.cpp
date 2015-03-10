
#include <algorithm>

#include "http_message_head.hpp"

namespace IPSuite
{
  namespace HTTP
  {
    //----------------------------------------------------------------//
    MessageHead::MessageHead()
    {

    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    MessageHead::~MessageHead()
    {
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void MessageHead::header(const std::string& name, const std::string& value)
    {
      std::string n(name);
      std::string v(value);
      this->header(std::move(n), std::move(v));
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void MessageHead::header(std::string&& name, std::string&& value)
    {
      // trim
      const std::string whitespace(" \t\f\v\r\n");
      name.erase(0, name.find_first_not_of(whitespace));
      name.erase(name.find_last_not_of(whitespace)+1);
      value.erase(0, value.find_first_not_of(whitespace));
      value.erase(value.find_last_not_of(whitespace)+1);

      // make name lowercase
      std::for_each(name.begin(), name.end(), ::tolower);

      for (auto it = this->headers_.begin(); it != this->headers_.end();)
      {
        if (it->first == name)
          it = this->headers_.erase(it);
        else
          ++it;
      }
      this->headers_.push_back(std::pair<std::string,std::string>(std::move(name), std::move(value)));
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void MessageHead::multiHeader(const std::string& name, const std::list<std::string>& values)
    {
      std::string n(name);
      std::list<std::string> v(values);
      this->multiHeader(std::move(n), std::move(v));
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void MessageHead::multiHeader(std::string&& name, std::list<std::string>&& values)
    {
      // trim
      const std::string whitespace(" \t\f\v\r\n");
      name.erase(0, name.find_first_not_of(whitespace));
      name.erase(name.find_last_not_of(whitespace)+1);

      // make name lowercase
      std::for_each(name.begin(), name.end(), ::tolower);

      for (auto it = this->headers_.begin(); it != this->headers_.end();)
      {
        if (it->first == name)
          it = this->headers_.erase(it);
        else
          ++it;
      }

      std::for_each(values.begin(), values.end(), [this, &whitespace, &name](std::string& value)
      {
        value.erase(0, value.find_first_not_of(whitespace));
        value.erase(value.find_last_not_of(whitespace)+1);

        this->headers_.push_back(std::pair<std::string,std::string>(std::move(name), std::move(value)));
      });

    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    std::string MessageHead::header(const std::string& name) const
    {
      std::string ret;
      std::string nameToLower(name);
      std::for_each(nameToLower.begin(), nameToLower.end(), ::tolower);

      for (auto it = this->headers_.rbegin(); ret.empty() && it != this->headers_.rend(); ++it)
      {
        if (it->first == nameToLower)
          ret = it->second;
      }

      return ret;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    std::list<std::string> MessageHead::multiHeader(const std::string& name) const
    {
      std::list<std::string> ret;
      std::string nameToLower(name);
      std::for_each(nameToLower.begin(), nameToLower.end(), ::tolower);

      for (auto it = this->headers_.begin(); it != this->headers_.end(); ++it)
      {
        if (it->first == nameToLower)
          ret.push_back(it->second);
      }

      return ret;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    const std::string&  MessageHead::httpVersion() const
    {
      return this->version_;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void  MessageHead::httpVersion(const std::string& version)
    {
      this->version_ = version;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void MessageHead::serialize(const MessageHead& source, std::string& destination)
    {
      destination = source.startLine() + "\r\n";
      for (auto it = source.headers_.begin(); it != source.headers_.end(); ++it)
      {
        destination += it->first;
        destination += ": ";
        destination += it->second;
        destination += "\r\n";
      }
      destination += "\r\n";
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    bool MessageHead::deserialize(const std::string& source, MessageHead& destination)
    {
      bool ret = false;
      std::size_t pos = 0;
      pos = source.find("\r\n", pos);
      if (pos != std::string::npos)
      {
        destination.startLine(source.substr(0, pos));
        pos += 2;

        while (!ret && pos != source.size())
        {
          std::size_t endPos = source.find("\r\n", pos);
          if (endPos == std::string::npos)
          {
            break;
          }
          else if (endPos == pos)
          {
            ret = true;
          }
          else
          {
            const char * delim = ":";
            const char* colonPtr = std::search(&source[pos], &source[pos] + endPos, delim, delim + 1);
            if (colonPtr == &source[pos] + endPos)
            {
              break;
            }
            else
            {
              std::string name(&source[pos], colonPtr - &source[pos]);
              ++colonPtr;
              std::string value(colonPtr, &source[endPos] - colonPtr);

              destination.header(std::move(name), std::move(value)); // header method trims.

              pos = endPos + 2;
            }
          }
        }

      }

      return ret;
    }
    //----------------------------------------------------------------//
  }
}