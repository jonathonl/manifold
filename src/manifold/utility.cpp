// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "manifold/utility.hpp"

#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif
#include <map>


namespace manifold
{
  namespace detail
  {
    bool path_exists(const std::string& input_path)
    {
      struct stat s;
      return (stat(input_path.c_str(), &s) == 0);
    }

    bool is_regular_file(const std::string& input_path)
    {
      struct stat s;
      return (stat(input_path.c_str(), &s) == 0 && s.st_mode & S_IFREG);
    }

    bool is_directory(const std::string& input_path)
    {
      struct stat s;
      return (stat(input_path.c_str(), &s) == 0 && s.st_mode & S_IFDIR);
    }

    std::string basename(const std::string& input_path)
    {
      return input_path.substr(input_path.find_last_of("/\\") + 1);
    }

    std::string basename_sans_extension(const std::string& input_path)
    {
      std::string ret = basename(input_path);

      if (ret.size() && ret.front() == '.')
      {
        std::string tmp = ret.substr(1);
        ret = "." + tmp.substr(0, tmp.find_last_of("."));
      }
      else
      {
        ret = ret.substr(0, ret.find_last_of("."));
      }

      return ret;
    }

    std::string extension(const std::string& input_path)
    {
      std::string ret;
      if (input_path.size() && input_path.front() == '.')
      {
        std::string tmp = input_path.substr(1);
        auto pos = tmp.find_last_of(".");
        if (pos != std::string::npos)
          ret = tmp.substr(pos);
      }
      else
      {
        auto pos = input_path.find_last_of(".");
        if (pos != std::string::npos)
          ret = input_path.substr(pos);
      }
      return ret;
    }

    std::string directory(const std::string& input_path)
    {
      std::string ret = input_path;
      if (ret == "." || ret == "..")
        ret += "/";
      ret.erase(ret.find_last_of("/\\") + 1);
      return ret;
    }

    const std::map<std::string, std::string> content_type_index =
      {
        {".json", "application/json"},
        {".js",   "application/javascript"},
        {".html", "text/html"},
        {".htm",  "text/html"},
        {".css",  "text/css"},
        {".xml",  "text/xml"},
        {".txt",  "text/plain"},
        {".md",   "text/markdown"}
      };

    std::string content_type_from_extension(const std::string& extension)
    {
      std::string ret;

      auto it = content_type_index.find(extension);
      if (it != content_type_index.end())
        ret = it->second;

      return ret;
    }
  }
}