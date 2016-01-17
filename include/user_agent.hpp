#pragma once
#ifndef MANIFOLD_USER_AGENT_HPP
#define MANIFOLD_USER_AGENT_HPP

#include "uniform_resource_identifier.hpp"
#include "http_client.hpp"

#include <map>
#include <set>
#include <memory>

namespace manifold
{
  namespace http
  {
    class user_agent_promise;

    class user_agent
    {
    public:
      user_agent(asio::io_service& ioservice);
      ~user_agent();


      class request
      {
      public:
        request(const std::shared_ptr<user_agent_promise>& prom);
        request(request&& source);
        ~request();

        void on_response(const std::function<void(client::response&& resp)>& cb);
      private:
        std::shared_ptr<user_agent_promise> promise_;
      };

      client& get_client(const uri& request_url);
      request send_request(const std::string& meth, const uri& url, std::stringstream&& entity);
    private:
      asio::io_service& io_service_;
      std::map<std::string, client> clients_;
      std::set<std::shared_ptr<user_agent_promise>> request_promises_;
    };
  }
}


#endif //MANIFOLD_USER_AGENT_HPP
