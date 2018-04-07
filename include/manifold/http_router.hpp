#pragma once

#ifndef MANIFOLD_HTTP_ROUTER_HPP
#define MANIFOLD_HTTP_ROUTER_HPP

#include <list>
#include <regex>

#include "http_server.hpp"


namespace manifold
{
  namespace http
  {
    //================================================================//
    class router
    {
    private:
      //================================================================//
      struct route
      {
        std::regex expression;
        std::function<future<void>(server::request req, server::response res, std::smatch matches)> handler;
        std::string method;
        route(const std::regex& exp, const std::function<future<void>(server::request req, server::response res, std::smatch matches)>& fn, const std::string& meth = "")
          : expression(exp), handler(fn), method(meth)
        {}
      };
      //================================================================//

      //----------------------------------------------------------------//
      std::list<route> routes_;
      //----------------------------------------------------------------//
    public:
      //----------------------------------------------------------------//
      void register_handler(const std::regex& expression, const std::function<future<void>(server::request req, server::response res, std::smatch matches)>& handler);
      void register_handler(const std::regex& expression, const std::string& method, const std::function<future<void>(server::request req, server::response res, std::smatch matches)>& handler);
      future<void> route(server::request req, server::response res);
      future<void> operator()(server::request req, server::response res);
      //----------------------------------------------------------------//
    };
    //================================================================//
  }
}

#endif // MANIFOLD_HTTP_ROUTER_HPP