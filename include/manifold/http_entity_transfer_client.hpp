#pragma once
#ifndef MANIFOLD_HTTP_STREAM_CLIENT_HPP
#define MANIFOLD_HTTP_STREAM_CLIENT_HPP

#include "http_client.hpp"
#include "uniform_resource_identifier.hpp"

#include <iostream>

namespace manifold
{
  namespace http
  {
    enum class response_status_errc : std::uint16_t
    {
      unknown_redirection_status = 1,
      unknown_client_error,
      unknown_server_error,

      // redirection
        multiple_choices                = 300,
      moved_permanently               = 301,
      found                           = 302,
      see_other                       = 303,
      not_modified                    = 304,
      use_proxy                       = 305,

      temporary_redirect              = 307,

      // client error
        bad_request                     = 400,
      unauthorized                    = 401,
      payment_required                = 402,
      forbidden                       = 403,
      not_found                       = 404,
      method_not_allowed              = 405,
      not_acceptable                  = 406,
      proxy_authentication_required   = 407,
      request_timeout                 = 408,
      conflict                        = 409,
      gone                            = 410,
      length_required                 = 411,
      precondition_failed             = 412,
      request_entity_too_large        = 413,
      request_uri_too_long            = 414,
      unsupported_media_type          = 415,
      requested_range_not_satisfiable = 416,
      expectation_failed              = 417,

      // server error
        internal_server_error           = 500,
      not_implemented                 = 501,
      bad_gateway                     = 502,
      service_unavailable             = 503,
      gateway_timeout                 = 504,
      http_version_not_supported      = 505
    };
  }
}

namespace std
{
  template<> struct is_error_code_enum<manifold::http::response_status_errc> : public true_type {};
  std::error_code make_error_code(manifold::http::response_status_errc e);
}

namespace manifold
{
  namespace http
  {
    class response_status_error_category_impl : public std::error_category
    {
    public:
      response_status_error_category_impl() {}
      ~response_status_error_category_impl() {}
      const char* name() const noexcept;
      std::string message(int ev) const;
    };

    class entity_transfer
    {
    public:
      entity_transfer(uri remote_url);
      virtual ~entity_transfer() {}

      const uri& remote_url() const;
      std::list<std::pair<std::string,std::string>>& request_headers();
    private:
      uri remote_url_;
      std::list<std::pair<std::string,std::string>> request_headers_;
    };

    class ios_transfer : public entity_transfer
    {
    public:
      ios_transfer(uri remote_url, std::string request_method, std::istream& request_entity, std::ostream& response_entity);
      ios_transfer(uri remote_url, std::string request_method, std::istream& request_entity);
      ios_transfer(uri remote_url, std::string request_method, std::ostream& response_entity);
      ios_transfer(uri remote_url, std::string request_method);

      const std::string& request_method() const;
      std::istream* request_entity();
      std::ostream* response_entity();
    private:
      std::string request_method_;
      std::istream* request_entity_ = nullptr;
      std::ostream* response_entity_ = nullptr;
    };

    class file_download : public entity_transfer
    {
    public:
      file_download(uri remote_source, std::string local_destination);
      const std::string& local_destination() const;
    private:
      std::string local_destination_;
    };

    class file_upload : public entity_transfer
    {
    public:
      file_upload(uri remote_destination, std::string local_source);
      const std::string& local_source() const;
    private:
      std::string local_source_;
    };

    class remote_file_stat : public entity_transfer
    {
    public:
      struct statistics
      {
        std::int64_t file_size;
        std::string mime_type;
        std::string modification_date;
        // TODO: cache expire
      };

      remote_file_stat(uri remote_file);
    };

    class entity_transfer_client
    {
    public:
      typedef std::function<void(std::uint64_t, std::uint64_t, std::ios::openmode direction)> progress_callback;

      entity_transfer_client(asio::io_service& io_ctx, asio::ssl::context& ssl_ctx);
      ~entity_transfer_client();

//      future<response_head> send_request(const std::string& method, const uri& request_url, std::ostream& res_entity, std::error_code& ec);
//      future<response_head> send_request(const std::string& method, const uri& request_url, std::list<std::pair<std::string,std::string>> header_list, std::ostream& res_entity, std::error_code& ec);
//      future<response_head> send_request(const std::string& method, const uri& request_url, std::istream& req_entity, std::ostream& res_entity, std::error_code& ec);
//      future<response_head> send_request(const std::string& method, const uri& request_url, std::list<std::pair<std::string,std::string>> header_list, std::istream& req_entity, std::ostream& res_entity, std::error_code& ec);


      future<response_head> operator()(ios_transfer& transfer, std::error_code& ec);
      future<void> operator()(file_download& transfer, std::error_code& ec);
      future<void> operator()(file_upload& transfer, std::error_code& ec);
      future<remote_file_stat::statistics> operator()(remote_file_stat& transfer, std::error_code& ec);

      void reset_max_redirects(std::uint8_t value = 5);
    private:
      future<response_head> run_transfer(const std::string& method, uri request_url, std::error_code& ec, std::list<std::pair<std::string,std::string>> header_list = {}, std::istream* req_entity = nullptr, std::ostream* res_entity = nullptr, progress_callback progress = nullptr);
    private:
      asio::ssl::context& ssl_ctx_;
      client client_;
      client secure_client_;
      std::uint8_t max_redirects_;
      std::mt19937 rng_;
    };
  }
}

#endif //MANIFOLD_HTTP_STREAM_CLIENT_HPP
