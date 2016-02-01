
#include "http_stream_client.hpp"

namespace manifold
{
  namespace http
  {
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
              prom->fulfill(std::make_error_code(std::errc::protocol_error), resp.head()); // TODO: set custom error.
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
            prom->fulfill(std::make_error_code(std::errc::protocol_error), resp.head());
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