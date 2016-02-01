#pragma once
#ifndef MANIFOLD_HTTP_STREAM_CLIENT_HPP
#define MANIFOLD_HTTP_STREAM_CLIENT_HPP

#include "http_client.hpp"
#include "uniform_resource_identifier.hpp"

namespace manifold
{
  namespace http
  {
    class stream_client
    {
    private:
      class promise_impl
      {
      public:
        void fulfill(const std::error_code& ec, const response_head& headers);
        void cancel();
        void on_complete(const std::function<void(const std::error_code& ec, const response_head& headers)>& fn);
        void on_cancel(const std::function<void()>& fn);
      private:
        bool fulfilled_ = false;
        bool cancelled_ = false;
        std::function<void(const std::error_code& ec, const response_head& headers)> on_complete_;
        std::function<void()> on_cancel_;
        std::error_code ec_;
        response_head headers_;
      };
    public:
      class promise
      {
      public:
        promise(const std::shared_ptr<promise_impl>& impl);
        void on_complete(const std::function<void(const std::error_code& ec, const response_head& res_head)>& fn);
        void cancel();
      private:
        std::shared_ptr<promise_impl> impl_;
      };

      stream_client(client& c);
      ~stream_client();

      promise send_request(const std::string& method, const uri& request_url, std::ostream& res_entity, std::uint8_t max_redirects = 5);
      promise send_request(const std::string& method, const uri& request_url, const std::list<std::pair<std::string,std::string>>& header_list, std::ostream& res_entity, std::uint8_t max_redirects = 5);
      promise send_request(const std::string& method, const uri& request_url, std::istream& req_entity, std::ostream& res_entity, std::uint8_t max_redirects = 5);
      promise send_request(const std::string& method, const uri& request_url, const std::list<std::pair<std::string,std::string>>& header_list, std::istream& req_entity, std::ostream& res_entity, std::uint8_t max_redirects = 5);
    private:
      client& client_;

      void handle_request(const std::error_code& ec, client::request&& req, const std::string& method, const uri& request_url, const std::list<std::pair<std::string,std::string>>& header_list, std::istream* req_entity, std::ostream* resp_entity, std::uint8_t max_redirects, const std::shared_ptr<promise_impl>& prom);
    };
  }
}

#endif //MANIFOLD_HTTP_STREAM_CLIENT_HPP
