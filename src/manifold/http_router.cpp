// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "manifold/http_router.hpp"
#include "manifold/uniform_resource_identifier.hpp"


namespace manifold
{
  namespace http
  {
    //----------------------------------------------------------------//
    void router::register_handler(const std::regex& expression, const std::function<future<void>(server::request req, server::response res, std::smatch matches)>& handler)
    {
      if (handler)
        this->routes_.emplace_back(expression, handler);
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void router::register_handler(const std::regex& expression, const std::string& method, const std::function<future<void>(server::request req, server::response res, std::smatch matches)>& handler)
    {
      if (handler)
        this->routes_.emplace_back(expression, handler, method);
    }
    //----------------------------------------------------------------//

    future<void> router::route(server::request req, server::response res)
    {
      return router::operator()(std::move(req), std::move(res));
    }

    //----------------------------------------------------------------//
    future<void> router::operator()(server::request req, server::response res)
    {
      bool both_matched = false;
      bool path_matched = false;

      std::string request_path = percent_decode(uri(req.head().path()).path());
      std::string request_method = req.head().method();
      std::smatch sm;

      for (auto rt = this->routes_.begin(); !both_matched && rt != this->routes_.end(); ++rt)
      {
        if (std::regex_match(request_path, sm, rt->expression))
        {
          if (rt->method.empty() || rt->method == request_method)
          {
            rt->handler(std::move(req), std::move(res), sm);
            both_matched = true;
          }
          path_matched = true;
        }
      }

      if (!both_matched)
      {
        if (path_matched)
        {
          res.head().set_status_code(status_code::method_not_allowed);
          res.end();
        }
        else
        {
          res.head().set_status_code(status_code::not_found);
          res.end();
        }
      }
      co_return;
    }
    //----------------------------------------------------------------//
  }
}