
#include "http_stream_client.hpp"

namespace manifold
{
  namespace http
  {
    //================================================================//
    stream_client_errc status_code_to_errc(std::uint16_t status_code)
    {
      if (status_code >= 300 && status_code < 400)
      {
        switch (status_code)
        {
          case static_cast<std::uint16_t>(stream_client_errc::response_status_multiple_choices  ): return stream_client_errc::response_status_multiple_choices   ;
          case static_cast<std::uint16_t>(stream_client_errc::response_status_moved_permanently ): return stream_client_errc::response_status_moved_permanently  ;
          case static_cast<std::uint16_t>(stream_client_errc::response_status_found             ): return stream_client_errc::response_status_found              ;
          case static_cast<std::uint16_t>(stream_client_errc::response_status_see_other         ): return stream_client_errc::response_status_see_other          ;
          case static_cast<std::uint16_t>(stream_client_errc::response_status_not_modified      ): return stream_client_errc::response_status_not_modified       ;
          case static_cast<std::uint16_t>(stream_client_errc::response_status_use_proxy         ): return stream_client_errc::response_status_use_proxy          ;
          case static_cast<std::uint16_t>(stream_client_errc::response_status_temporary_redirect): return stream_client_errc::response_status_temporary_redirect ;
          default: return stream_client_errc::unknown_redirection_response_status;
        }
      }
      else if (status_code >= 400 && status_code < 500)
      {
        switch (status_code)
        {
          case static_cast<std::uint16_t>(stream_client_errc::response_status_bad_request                    ): return stream_client_errc::response_status_bad_request                     ;
          case static_cast<std::uint16_t>(stream_client_errc::response_status_unauthorized                   ): return stream_client_errc::response_status_unauthorized                    ;
          case static_cast<std::uint16_t>(stream_client_errc::response_status_payment_required               ): return stream_client_errc::response_status_payment_required                ;
          case static_cast<std::uint16_t>(stream_client_errc::response_status_forbidden                      ): return stream_client_errc::response_status_forbidden                       ;
          case static_cast<std::uint16_t>(stream_client_errc::response_status_not_found                      ): return stream_client_errc::response_status_not_found                       ;
          case static_cast<std::uint16_t>(stream_client_errc::response_status_method_not_allowed             ): return stream_client_errc::response_status_method_not_allowed              ;
          case static_cast<std::uint16_t>(stream_client_errc::response_status_not_acceptable                 ): return stream_client_errc::response_status_not_acceptable                  ;
          case static_cast<std::uint16_t>(stream_client_errc::response_status_proxy_authentication_required  ): return stream_client_errc::response_status_proxy_authentication_required   ;
          case static_cast<std::uint16_t>(stream_client_errc::response_status_request_timeout                ): return stream_client_errc::response_status_request_timeout                 ;
          case static_cast<std::uint16_t>(stream_client_errc::response_status_conflict                       ): return stream_client_errc::response_status_conflict                        ;
          case static_cast<std::uint16_t>(stream_client_errc::response_status_gone                           ): return stream_client_errc::response_status_gone                            ;
          case static_cast<std::uint16_t>(stream_client_errc::response_status_length_required                ): return stream_client_errc::response_status_length_required                 ;
          case static_cast<std::uint16_t>(stream_client_errc::response_status_precondition_failed            ): return stream_client_errc::response_status_precondition_failed             ;
          case static_cast<std::uint16_t>(stream_client_errc::response_status_request_entity_too_large       ): return stream_client_errc::response_status_request_entity_too_large        ;
          case static_cast<std::uint16_t>(stream_client_errc::response_status_request_uri_too_long           ): return stream_client_errc::response_status_request_uri_too_long            ;
          case static_cast<std::uint16_t>(stream_client_errc::response_status_unsupported_media_type         ): return stream_client_errc::response_status_unsupported_media_type          ;
          case static_cast<std::uint16_t>(stream_client_errc::response_status_requested_range_not_satisfiable): return stream_client_errc::response_status_requested_range_not_satisfiable ;
          case static_cast<std::uint16_t>(stream_client_errc::response_status_expectation_failed             ): return stream_client_errc::response_status_expectation_failed              ;
          default: return stream_client_errc::unknown_client_error_response_status;
        }
      }
      else if (status_code >= 500 && status_code < 600)
      {
        switch (status_code)
        {
          case static_cast<std::uint16_t>(stream_client_errc::response_status_internal_server_error     ): return stream_client_errc::response_status_internal_server_error      ;
          case static_cast<std::uint16_t>(stream_client_errc::response_status_not_implemented           ): return stream_client_errc::response_status_not_implemented            ;
          case static_cast<std::uint16_t>(stream_client_errc::response_status_bad_gateway               ): return stream_client_errc::response_status_bad_gateway                ;
          case static_cast<std::uint16_t>(stream_client_errc::response_status_service_unavailable       ): return stream_client_errc::response_status_service_unavailable        ;
          case static_cast<std::uint16_t>(stream_client_errc::response_status_gateway_timeout           ): return stream_client_errc::response_status_gateway_timeout            ;
          case static_cast<std::uint16_t>(stream_client_errc::response_status_http_version_not_supported): return stream_client_errc::response_status_http_version_not_supported ;
          default: return stream_client_errc::unknown_server_error_response_status;
        }
      }
      return stream_client_errc::unknown_server_error_response_status;
    }

    const char* stream_client_error_category_impl::name() const noexcept
    {
      return "Manifold HTTP Stream Clientr";
    }

    std::string stream_client_error_category_impl::message(int ev) const
    {
      switch (ev)
      {
        case static_cast<int>(stream_client_errc::unknown_redirection_response_status             ): return "unknown redirection response status";
        case static_cast<int>(stream_client_errc::unknown_client_error_response_status            ): return "unknown client error response status";
        case static_cast<int>(stream_client_errc::unknown_server_error_response_status            ): return "unknown server error response status";
        case static_cast<int>(stream_client_errc::response_status_multiple_choices                ): return "response status multiple choices";
        case static_cast<int>(stream_client_errc::response_status_moved_permanently               ): return "response status moved permanently";
        case static_cast<int>(stream_client_errc::response_status_found                           ): return "response status found";
        case static_cast<int>(stream_client_errc::response_status_see_other                       ): return "response status see other";
        case static_cast<int>(stream_client_errc::response_status_not_modified                    ): return "response status not modified";
        case static_cast<int>(stream_client_errc::response_status_use_proxy                       ): return "response status use proxy";
        case static_cast<int>(stream_client_errc::response_status_temporary_redirect              ): return "response status temporary redirect";
        case static_cast<int>(stream_client_errc::response_status_bad_request                     ): return "response status bad request";
        case static_cast<int>(stream_client_errc::response_status_unauthorized                    ): return "response status unauthorized";
        case static_cast<int>(stream_client_errc::response_status_payment_required                ): return "response status payment required";
        case static_cast<int>(stream_client_errc::response_status_forbidden                       ): return "response status forbidden";
        case static_cast<int>(stream_client_errc::response_status_not_found                       ): return "response status not found";
        case static_cast<int>(stream_client_errc::response_status_method_not_allowed              ): return "response status method not allowed";
        case static_cast<int>(stream_client_errc::response_status_not_acceptable                  ): return "response status not acceptable";
        case static_cast<int>(stream_client_errc::response_status_proxy_authentication_required   ): return "response status proxy authentication required";
        case static_cast<int>(stream_client_errc::response_status_request_timeout                 ): return "response status request timeout";
        case static_cast<int>(stream_client_errc::response_status_conflict                        ): return "response status conflict";
        case static_cast<int>(stream_client_errc::response_status_gone                            ): return "response status gone";
        case static_cast<int>(stream_client_errc::response_status_length_required                 ): return "response status length required";
        case static_cast<int>(stream_client_errc::response_status_precondition_failed             ): return "response status precondition failed";
        case static_cast<int>(stream_client_errc::response_status_request_entity_too_large        ): return "response status request entity too large";
        case static_cast<int>(stream_client_errc::response_status_request_uri_too_long            ): return "response status request uri too long";
        case static_cast<int>(stream_client_errc::response_status_unsupported_media_type          ): return "response status unsupported media type";
        case static_cast<int>(stream_client_errc::response_status_requested_range_not_satisfiable ): return "response status requested range not satisfiable";
        case static_cast<int>(stream_client_errc::response_status_expectation_failed              ): return "response status expectation failed";
        case static_cast<int>(stream_client_errc::response_status_internal_server_error           ): return "response status internal server error";
        case static_cast<int>(stream_client_errc::response_status_not_implemented                 ): return "response status not implemented";
        case static_cast<int>(stream_client_errc::response_status_bad_gateway                     ): return "response status bad gateway";
        case static_cast<int>(stream_client_errc::response_status_service_unavailable             ): return "response status service unavailable";
        case static_cast<int>(stream_client_errc::response_status_gateway_timeout                 ): return "response status gateway timeout";
        case static_cast<int>(stream_client_errc::response_status_http_version_not_supported      ): return "response status http version not supported";
      };
      return "Unknown Error";
    }

    const manifold::http::stream_client_error_category_impl stream_client_error_category_object;
    std::error_code make_error_code (manifold::http::stream_client_errc e)
    {
      return std::error_code(static_cast<int>(e), stream_client_error_category_object);
    }
    //================================================================//

    //================================================================//
    void stream_client::promise_impl::fulfill(const std::error_code& ec, const response_head& headers)
    {
      if (!fulfilled_)
      {
        fulfilled_ = true;

        ec_ = ec;
        headers_ = headers;

        on_complete_ ? on_complete_(ec_, headers_) : void();
        on_complete_ = nullptr;
      }
    }

    void stream_client::promise_impl::cancel()
    {
      if (!cancelled_)
      {
        cancelled_ = true;

        on_cancel_ ? on_cancel_() : void();
        on_cancel_ = nullptr;
      }
    }

    void stream_client::promise_impl::on_complete(const std::function<void(const std::error_code& ec, const response_head& headers)>& fn)
    {
      if (fulfilled_)
        fn ? fn(ec_, headers_) : void();
      else
        on_complete_ = fn;
    }

    void stream_client::promise_impl::on_cancel(const std::function<void()>& fn)
    {
      if (cancelled_)
        fn ? fn() : void();
      else
        on_cancel_ = fn;
    }
    //================================================================//

    //================================================================//
    stream_client::promise::promise(const std::shared_ptr<promise_impl>& impl)
      : impl_(impl)
    {
    }

    void stream_client::promise::on_complete(const std::function<void(const std::error_code& ec, const response_head& res_head)>& fn)
    {
      impl_->on_complete(fn);
    }

    void stream_client::promise::cancel()
    {
      impl_->cancel();
    }
    //================================================================//

    //================================================================//
    stream_client::stream_client(client& c)
      : client_(c)
    {
      this->reset_max_redirects();
    }

    stream_client::~stream_client()
    {

    }

    void stream_client::reset_max_redirects(std::uint8_t value)
    {
      this->max_redirects_ = value;
    }

    stream_client::promise stream_client::send_request(const std::string& method, const uri& request_url, std::ostream& res_entity)
    {
      return send_request(method, request_url, res_entity, max_redirects_);
    }

    stream_client::promise stream_client::send_request(const std::string& method, const uri& request_url, const std::list<std::pair<std::string,std::string>>& header_list, std::ostream& res_entity)
    {
      return send_request(method, request_url, header_list, res_entity, max_redirects_);
    }

    stream_client::promise stream_client::send_request(const std::string& method, const uri& request_url, std::istream& req_entity, std::ostream& res_entity)
    {
      return send_request(method, request_url, req_entity, res_entity, max_redirects_);
    }

    stream_client::promise stream_client::send_request(const std::string& method, const uri& request_url, const std::list<std::pair<std::string,std::string>>& header_list, std::istream& req_entity, std::ostream& res_entity)
    {
      return send_request(method, request_url, header_list, req_entity, res_entity, max_redirects_);
    }

    void stream_client::handle_request(const std::error_code& ec, client::request&& tmp_req, const std::string& method, const uri& request_url, const std::list<std::pair<std::string,std::string>>& header_list, std::istream* req_entity, std::ostream* resp_entity, std::uint8_t max_redirects, const std::shared_ptr<promise_impl>& prom)
    {
      if (ec)
      {
        prom->fulfill(ec, response_head());
      }
      else
      {
        auto req = std::make_shared<client::request>(std::move(tmp_req));

        req->on_close([prom](const std::error_code& ec)
        {
          prom->fulfill(ec, response_head());
        });

        prom->on_cancel([req]()
        {
          req->cancel();
        });

        req->on_response([this, prom, req_entity, resp_entity, max_redirects, method, request_url, header_list, req](client::response&& resp)
        {
          if (resp.head().has_successful_status())
          {
            if (resp_entity)
            {
              resp.on_data([resp_entity](const char*const data, std::size_t sz)
              {
                resp_entity->write(data, sz);
              });
            }

            auto headers = std::make_shared<response_head>(resp.head());
            resp.on_end([prom, headers]()
            {
              prom->fulfill(std::error_code(), *headers);
            });
          }
          else if (resp.head().has_redirection_status())
          {
            uri redirect_url = resp.head().header("location");
            if (!redirect_url.is_valid() || max_redirects == 0)
              prom->fulfill(status_code_to_errc(resp.head().status_code()), resp.head()); // TODO: set custom error.
            else
            {
              if (redirect_url.is_relative())
              {
                redirect_url.host(request_url.host());
                redirect_url.port(request_url.port());
              }

              if (redirect_url.scheme_name().empty())
                redirect_url.scheme_name(request_url.scheme_name());

              prom->on_cancel(nullptr);
              req->on_close(nullptr);

              if (request_url.scheme_name() == "https")
                this->client_.make_secure_request(request_url.host(), request_url.port(), std::bind(&stream_client::handle_request, this, std::placeholders::_1, std::placeholders::_2, method, redirect_url, header_list, req_entity, resp_entity, max_redirects - 1, prom));
              else
                this->client_.make_request(request_url.host(), request_url.port(), std::bind(&stream_client::handle_request, this, std::placeholders::_1, std::placeholders::_2, method, redirect_url, header_list, req_entity, resp_entity, max_redirects - 1, prom));
            }
          }
          else
          {
            prom->fulfill(status_code_to_errc(resp.head().status_code()), resp.head());
          }
        });

        req->head().method(method);
        req->head().path(request_url.path_with_query());
        for (auto it = header_list.begin(); it != header_list.end(); ++it)
          req->head().header(it->first, it->second);

        if (!req_entity)
        {
          req->end();
        }
        else
        {
          req_entity->clear();
          req_entity->seekg(0, std::ios::beg);

          std::array<char, 4096> buf;
          long bytes_in_buf = req_entity->read(buf.data(), buf.size()).gcount();
          if (!req_entity->good())
          {
            if (bytes_in_buf > 0)
              req->end(buf.data(), (std::size_t)bytes_in_buf);
            else
              req->end();
          }
          else
          {
            req->on_drain([req_entity, req]()
            {
              std::array<char, 4096> buf;
              long bytes_in_buf = req_entity->read(buf.data(), buf.size()).gcount();
              if (bytes_in_buf > 0)
                req->send(buf.data(), (std::size_t)bytes_in_buf);

              if (!req_entity->good())
                req->end();
            });
            req->send(buf.data(), (std::size_t)bytes_in_buf);
          }
        }
      }
    }

    stream_client::promise stream_client::send_request(const std::string& method, const uri& request_url, std::ostream& resp_entity, std::uint8_t max_redirects)
    {
      return this->send_request(method, request_url, {}, resp_entity, max_redirects);
    }

    stream_client::promise stream_client::send_request(const std::string& method, const uri& request_url, const std::list<std::pair<std::string,std::string>>& header_list, std::ostream& resp_entity, std::uint8_t max_redirects)
    {
      auto prom = std::make_shared<promise_impl>();
      promise ret(prom);

      if (request_url.scheme_name() == "https")
        this->client_.make_secure_request(request_url.host(), request_url.port(), std::bind(&stream_client::handle_request, this, std::placeholders::_1, std::placeholders::_2, method, request_url, header_list, nullptr, &resp_entity, max_redirects, prom));
      else
        this->client_.make_request(request_url.host(), request_url.port(), std::bind(&stream_client::handle_request, this, std::placeholders::_1, std::placeholders::_2, method, request_url, header_list, nullptr, &resp_entity, max_redirects, prom));

      return ret;
    }

    stream_client::promise stream_client::send_request(const std::string& method, const uri& request_url, std::istream& req_entity, std::ostream& res_entity, std::uint8_t max_redirects)
    {
      return this->send_request(method, request_url, {}, req_entity, res_entity, max_redirects);
    }

    stream_client::promise stream_client::send_request(const std::string& method, const uri& request_url, const std::list<std::pair<std::string,std::string>>& header_list, std::istream& req_entity, std::ostream& resp_entity, std::uint8_t max_redirects)
    {
      auto prom = std::make_shared<promise_impl>();
      promise ret(prom);

      if (request_url.scheme_name() == "https")
        this->client_.make_secure_request(request_url.host(), request_url.port(), std::bind(&stream_client::handle_request, this, std::placeholders::_1, std::placeholders::_2, method, request_url, header_list, &req_entity, &resp_entity, max_redirects, prom));
      else
        this->client_.make_request(request_url.host(), request_url.port(), std::bind(&stream_client::handle_request, this, std::placeholders::_1, std::placeholders::_2, method, request_url, header_list, &req_entity, &resp_entity, max_redirects, prom));

      return ret;
    }
    //================================================================//
  }
}