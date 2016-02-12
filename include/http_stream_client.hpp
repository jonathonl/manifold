#pragma once
#ifndef MANIFOLD_HTTP_STREAM_CLIENT_HPP
#define MANIFOLD_HTTP_STREAM_CLIENT_HPP

#include "http_client.hpp"
#include "uniform_resource_identifier.hpp"

namespace manifold
{
  namespace http
  {
    enum class stream_client_errc : std::uint16_t
    {
      unknown_redirection_response_status = 1,
      unknown_client_error_response_status,
      unknown_server_error_response_status,

      // redirection
      response_status_multiple_choices                = 300,
      response_status_moved_permanently               = 301,
      response_status_found                           = 302,
      response_status_see_other                       = 303,
      response_status_not_modified                    = 304,
      response_status_use_proxy                       = 305,

      response_status_temporary_redirect              = 307,

      // client error
      response_status_bad_request                     = 400,
      response_status_unauthorized                    = 401,
      response_status_payment_required                = 402,
      response_status_forbidden                       = 403,
      response_status_not_found                       = 404,
      response_status_method_not_allowed              = 405,
      response_status_not_acceptable                  = 406,
      response_status_proxy_authentication_required   = 407,
      response_status_request_timeout                 = 408,
      response_status_conflict                        = 409,
      response_status_gone                            = 410,
      response_status_length_required                 = 411,
      response_status_precondition_failed             = 412,
      response_status_request_entity_too_large        = 413,
      response_status_request_uri_too_long            = 414,
      response_status_unsupported_media_type          = 415,
      response_status_requested_range_not_satisfiable = 416,
      response_status_expectation_failed              = 417,

      // server error
      response_status_internal_server_error           = 500,
      response_status_not_implemented                 = 501,
      response_status_bad_gateway                     = 502,
      response_status_service_unavailable             = 503,
      response_status_gateway_timeout                 = 504,
      response_status_http_version_not_supported      = 505
    };
  }
}

namespace std
{
  template<> struct is_error_code_enum<manifold::http::stream_client_errc> : public true_type {};
}

namespace manifold
{
  namespace http
  {
    class stream_client_error_category_impl : public std::error_category
    {
    public:
      stream_client_error_category_impl() {}
      ~stream_client_error_category_impl() {}
      const char* name() const noexcept;
      std::string message(int ev) const;
    };

    std::error_code make_error_code(manifold::http::stream_client_errc e);

    class stream_client
    {
    public:
      typedef std::function<void(std::uint64_t, std::uint64_t)> progress_callback;
    private:
      class promise_impl
      {
      public:
        void update_send_progress(std::uint64_t, std::uint64_t);
        void update_recv_progress(std::uint64_t, std::uint64_t);
        void fulfill(const std::error_code&, const response_head&);
        void cancel();
        void on_send_progress(const progress_callback&);
        void on_recv_progress(const progress_callback&);
        void on_complete(const std::function<void(const std::error_code&, const response_head&)>& fn);
        void on_cancel(const std::function<void()>&);
      private:
        bool fulfilled_ = false;
        bool cancelled_ = false;
        std::function<void(const std::error_code& ec, const response_head& headers)> on_complete_;
        std::function<void()> on_cancel_;
        progress_callback on_send_progress_;
        progress_callback on_recv_progress_;
        std::error_code ec_;
        response_head headers_;
      };
    public:
      class promise
      {
      public:
        promise(const std::shared_ptr<promise_impl>& impl);
        void on_send_progress(const std::function<void(std::uint64_t bytes_transferred, std::uint64_t bytes_total)>& send_progress_cb);
        void on_recv_progress(const std::function<void(std::uint64_t bytes_transferred, std::uint64_t bytes_total)>& recv_progress_cb);
        void on_complete(const std::function<void(const std::error_code& ec, const response_head& res_head)>& fn);
        void cancel();
      private:
        std::shared_ptr<promise_impl> impl_;
      };

      stream_client(client& c);
      ~stream_client();

      promise send_request(const std::string& method, const uri& request_url, std::ostream& res_entity);
      promise send_request(const std::string& method, const uri& request_url, const std::list<std::pair<std::string,std::string>>& header_list, std::ostream& res_entity);
      promise send_request(const std::string& method, const uri& request_url, std::istream& req_entity, std::ostream& res_entity);
      promise send_request(const std::string& method, const uri& request_url, const std::list<std::pair<std::string,std::string>>& header_list, std::istream& req_entity, std::ostream& res_entity);

      void reset_max_redirects(std::uint8_t value = 5);
    private:
      client& client_;
      std::uint8_t max_redirects_;

      promise send_request(const std::string& method, const uri& request_url, const std::list<std::pair<std::string,std::string>>& header_list, std::ostream& res_entity, std::uint8_t max_redirects);
      promise send_request(const std::string& method, const uri& request_url, const std::list<std::pair<std::string,std::string>>& header_list, std::istream& req_entity, std::ostream& res_entity, std::uint8_t max_redirects);

      void handle_request(const std::error_code& ec, client::request&& req, const std::string& method, const uri& request_url, const std::list<std::pair<std::string,std::string>>& header_list, std::istream* req_entity, std::ostream* resp_entity, std::uint8_t max_redirects, const std::shared_ptr<promise_impl>& prom);
    };
  }
}

#endif //MANIFOLD_HTTP_STREAM_CLIENT_HPP
