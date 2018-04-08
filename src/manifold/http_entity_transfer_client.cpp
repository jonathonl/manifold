
#include "manifold/http_entity_transfer_client.hpp"
#include "manifold/utility.hpp"

#include <fstream>
#include <regex>

namespace manifold
{
  namespace http
  {
    //================================================================//
    response_status_errc status_code_to_errc(std::uint16_t status_code)
    {
      if (status_code >= 300 && status_code < 400)
      {
        switch (status_code)
        {
        case static_cast<std::uint16_t>(response_status_errc::multiple_choices  ): return response_status_errc::multiple_choices   ;
        case static_cast<std::uint16_t>(response_status_errc::moved_permanently ): return response_status_errc::moved_permanently  ;
        case static_cast<std::uint16_t>(response_status_errc::found             ): return response_status_errc::found              ;
        case static_cast<std::uint16_t>(response_status_errc::see_other         ): return response_status_errc::see_other          ;
        case static_cast<std::uint16_t>(response_status_errc::not_modified      ): return response_status_errc::not_modified       ;
        case static_cast<std::uint16_t>(response_status_errc::use_proxy         ): return response_status_errc::use_proxy          ;
        case static_cast<std::uint16_t>(response_status_errc::temporary_redirect): return response_status_errc::temporary_redirect ;
        default: return response_status_errc::unknown_redirection_status;
        }
      }
      else if (status_code >= 400 && status_code < 500)
      {
        switch (status_code)
        {
        case static_cast<std::uint16_t>(response_status_errc::bad_request                    ): return response_status_errc::bad_request                     ;
        case static_cast<std::uint16_t>(response_status_errc::unauthorized                   ): return response_status_errc::unauthorized                    ;
        case static_cast<std::uint16_t>(response_status_errc::payment_required               ): return response_status_errc::payment_required                ;
        case static_cast<std::uint16_t>(response_status_errc::forbidden                      ): return response_status_errc::forbidden                       ;
        case static_cast<std::uint16_t>(response_status_errc::not_found                      ): return response_status_errc::not_found                       ;
        case static_cast<std::uint16_t>(response_status_errc::method_not_allowed             ): return response_status_errc::method_not_allowed              ;
        case static_cast<std::uint16_t>(response_status_errc::not_acceptable                 ): return response_status_errc::not_acceptable                  ;
        case static_cast<std::uint16_t>(response_status_errc::proxy_authentication_required  ): return response_status_errc::proxy_authentication_required   ;
        case static_cast<std::uint16_t>(response_status_errc::request_timeout                ): return response_status_errc::request_timeout                 ;
        case static_cast<std::uint16_t>(response_status_errc::conflict                       ): return response_status_errc::conflict                        ;
        case static_cast<std::uint16_t>(response_status_errc::gone                           ): return response_status_errc::gone                            ;
        case static_cast<std::uint16_t>(response_status_errc::length_required                ): return response_status_errc::length_required                 ;
        case static_cast<std::uint16_t>(response_status_errc::precondition_failed            ): return response_status_errc::precondition_failed             ;
        case static_cast<std::uint16_t>(response_status_errc::request_entity_too_large       ): return response_status_errc::request_entity_too_large        ;
        case static_cast<std::uint16_t>(response_status_errc::request_uri_too_long           ): return response_status_errc::request_uri_too_long            ;
        case static_cast<std::uint16_t>(response_status_errc::unsupported_media_type         ): return response_status_errc::unsupported_media_type          ;
        case static_cast<std::uint16_t>(response_status_errc::requested_range_not_satisfiable): return response_status_errc::requested_range_not_satisfiable ;
        case static_cast<std::uint16_t>(response_status_errc::expectation_failed             ): return response_status_errc::expectation_failed              ;
        default: return response_status_errc::unknown_client_error;
        }
      }
      else if (status_code >= 500 && status_code < 600)
      {
        switch (status_code)
        {
        case static_cast<std::uint16_t>(response_status_errc::internal_server_error     ): return response_status_errc::internal_server_error      ;
        case static_cast<std::uint16_t>(response_status_errc::not_implemented           ): return response_status_errc::not_implemented            ;
        case static_cast<std::uint16_t>(response_status_errc::bad_gateway               ): return response_status_errc::bad_gateway                ;
        case static_cast<std::uint16_t>(response_status_errc::service_unavailable       ): return response_status_errc::service_unavailable        ;
        case static_cast<std::uint16_t>(response_status_errc::gateway_timeout           ): return response_status_errc::gateway_timeout            ;
        case static_cast<std::uint16_t>(response_status_errc::http_version_not_supported): return response_status_errc::http_version_not_supported ;
        default: return response_status_errc::unknown_server_error;
        }
      }
      return response_status_errc::unknown_server_error;
    }

    const char* response_status_error_category_impl::name() const noexcept
    {
      return "Manifold Unsuccessful HTTP Response Status";
    }

    std::string response_status_error_category_impl::message(int ev) const
    {
      switch (ev)
      {
      case static_cast<int>(response_status_errc::unknown_redirection_status      ): return "unknown redirection status";
      case static_cast<int>(response_status_errc::unknown_client_error            ): return "unknown client error";
      case static_cast<int>(response_status_errc::unknown_server_error            ): return "unknown server error";
      case static_cast<int>(response_status_errc::multiple_choices                ): return "multiple choices";
      case static_cast<int>(response_status_errc::moved_permanently               ): return "moved permanently";
      case static_cast<int>(response_status_errc::found                           ): return "found";
      case static_cast<int>(response_status_errc::see_other                       ): return "see other";
      case static_cast<int>(response_status_errc::not_modified                    ): return "not modified";
      case static_cast<int>(response_status_errc::use_proxy                       ): return "use proxy";
      case static_cast<int>(response_status_errc::temporary_redirect              ): return "temporary redirect";
      case static_cast<int>(response_status_errc::bad_request                     ): return "bad request";
      case static_cast<int>(response_status_errc::unauthorized                    ): return "unauthorized";
      case static_cast<int>(response_status_errc::payment_required                ): return "payment required";
      case static_cast<int>(response_status_errc::forbidden                       ): return "forbidden";
      case static_cast<int>(response_status_errc::not_found                       ): return "not found";
      case static_cast<int>(response_status_errc::method_not_allowed              ): return "method not allowed";
      case static_cast<int>(response_status_errc::not_acceptable                  ): return "not acceptable";
      case static_cast<int>(response_status_errc::proxy_authentication_required   ): return "proxy authentication required";
      case static_cast<int>(response_status_errc::request_timeout                 ): return "request timeout";
      case static_cast<int>(response_status_errc::conflict                        ): return "conflict";
      case static_cast<int>(response_status_errc::gone                            ): return "gone";
      case static_cast<int>(response_status_errc::length_required                 ): return "length required";
      case static_cast<int>(response_status_errc::precondition_failed             ): return "precondition failed";
      case static_cast<int>(response_status_errc::request_entity_too_large        ): return "request entity too large";
      case static_cast<int>(response_status_errc::request_uri_too_long            ): return "request uri too long";
      case static_cast<int>(response_status_errc::unsupported_media_type          ): return "unsupported media type";
      case static_cast<int>(response_status_errc::requested_range_not_satisfiable ): return "requested range not satisfiable";
      case static_cast<int>(response_status_errc::expectation_failed              ): return "expectation failed";
      case static_cast<int>(response_status_errc::internal_server_error           ): return "internal server error";
      case static_cast<int>(response_status_errc::not_implemented                 ): return "not implemented";
      case static_cast<int>(response_status_errc::bad_gateway                     ): return "bad gateway";
      case static_cast<int>(response_status_errc::service_unavailable             ): return "service unavailable";
      case static_cast<int>(response_status_errc::gateway_timeout                 ): return "gateway timeout";
      case static_cast<int>(response_status_errc::http_version_not_supported      ): return "http version not supported";
      };
      return "Unknown Error";
    }
  }
}


const manifold::http::response_status_error_category_impl stream_client_error_category_object;
std::error_code std::make_error_code(manifold::http::response_status_errc e)
{
  return std::error_code(static_cast<int>(e), stream_client_error_category_object);
}



namespace manifold
{
  namespace http
  {

    //================================================================//
    entity_transfer::entity_transfer(uri remote_url) :
      remote_url_(std::move(remote_url))
    {
    }

    const uri& entity_transfer::remote_url() const
    {
      return remote_url_;
    }

    std::list<std::pair<std::string,std::string>>& entity_transfer::request_headers()
    {
      return request_headers_;
    }

    void entity_transfer::update_progress(std::int64_t transfered, std::int64_t total, std::ios::openmode direction)
    {
      if (progress_callback_)
        progress_callback_(transfered, total, direction);
    }
    //================================================================//

    //================================================================//
    ios_transfer::ios_transfer(uri remote_url, std::string method, std::istream& request_entity, std::ostream& response_entity) :
      entity_transfer(std::move(remote_url)),
      request_entity_(&request_entity),
      response_entity_(&response_entity)
    {
    }

    ios_transfer::ios_transfer(uri remote_url, std::string method, std::istream& request_entity) :
      entity_transfer(std::move(remote_url)),
      request_entity_(&request_entity)
    {
    }

    ios_transfer::ios_transfer(uri remote_url, std::string method, std::ostream& response_entity) :
      entity_transfer(std::move(remote_url)),
      response_entity_(&response_entity)
    {
    }

    ios_transfer::ios_transfer(uri remote_url, std::string method) :
      entity_transfer(std::move(remote_url))
    {
    }

    const std::string& ios_transfer::request_method() const
    {
      return request_method_;
    }

    std::istream* ios_transfer::request_entity()
    {
      return request_entity_;
    }

    std::ostream* ios_transfer::response_entity()
    {
      return response_entity_;
    }
    //================================================================//

    //================================================================//
    file_download::file_download(uri remote_source, std::string local_destination) :
      entity_transfer(std::move(remote_source)),
      local_destination_(std::move(local_destination))
    {
    }

    const std::string& file_download::local_destination() const
    {
      return local_destination_;
    }

    void file_download::overwrite_existing(bool val)
    {
      overwrite_existing_ = val;
    }

    bool file_download::overwrite_existing() const
    {
       return overwrite_existing_;
    }
    //================================================================//

    //================================================================//
    file_upload::file_upload(uri remote_destination, std::string local_source) :
      entity_transfer(std::move(remote_destination)),
      local_source_(std::move(local_source))
    {
    }

    const std::string& file_upload::local_source() const
    {
      return local_source_;
    }
    //================================================================//

    //================================================================//
    remote_file_stat::remote_file_stat(uri remote_file) :
      entity_transfer(std::move(remote_file))
    {
    }
    //================================================================//

    //================================================================//
    entity_transfer_client::entity_transfer_client(asio::io_service& io_ctx, asio::ssl::context& ssl_ctx) :
      ssl_ctx_(ssl_ctx),
      client_(io_ctx),
      secure_client_(io_ctx, ssl_ctx_)
    {
      this->reset_max_redirects();
    }

    entity_transfer_client::~entity_transfer_client()
    {

    }

    void entity_transfer_client::reset_max_redirects(std::uint8_t value)
    {
      this->max_redirects_ = value;
    }

    future<response_head> entity_transfer_client::operator()(ios_transfer& transfer, std::error_code& ec)
    {
      return run_transfer(transfer.request_method(), transfer.remote_url(), ec, transfer.request_headers(), transfer.request_entity(), transfer.response_entity(), std::bind(&entity_transfer::update_progress, &transfer, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    }

    future<void> entity_transfer_client::operator()(file_download& transfer, std::error_code& ec)
    {
      std::string tmp_file_path;
      if (detail::is_directory(transfer.local_destination()))
      {
        tmp_file_path = transfer.local_destination();
        if (tmp_file_path.size() && (tmp_file_path.back() != '/' && tmp_file_path.back() != '\\'))
          tmp_file_path.push_back('/');
      }
      else
      {
        tmp_file_path = detail::directory(transfer.local_destination());
      }

      tmp_file_path += (detail::gen_uuid_str(this->rng_) + ".tmp");

      std::ofstream dest_ofs(tmp_file_path, std::ios::binary);

      if (!dest_ofs.good())
      {
        ec = std::error_code(errno, std::system_category());
      }
      else
      {
        if (transfer.remote_url().password().size() || transfer.remote_url().username().size())
          transfer.request_headers().emplace_back("authorization", basic_auth(transfer.remote_url().username(), transfer.remote_url().password()));


        auto resp_head = co_await run_transfer("GET", transfer.remote_url(), ec, transfer.request_headers(), nullptr, &dest_ofs, std::bind(&entity_transfer::update_progress, &transfer, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        dest_ofs.close();

        if (ec)
        {
          std::cerr << ec.value() << ": " << ec.message() << std::endl;
          std::remove(tmp_file_path.c_str());
          co_return;
        }
        else
        {
          std::string local_file_path = transfer.local_destination();
          std::replace(local_file_path.begin(), local_file_path.end(), '\\', '/');
          if (detail::is_directory(local_file_path))
          {
            if (local_file_path.size() && local_file_path.back() != '/')
              local_file_path += "/";
            std::string content_disposition = resp_head.header("content-disposition");
            std::string filename;
            std::regex exp(".*filename=(?:\"([^\"]*)\"|([^\\s;]*)).*", std::regex::ECMAScript);
            std::smatch sm;
            if (std::regex_match(content_disposition, sm, exp))
            {
              if (sm[1].matched)
                filename = detail::basename(sm[1].str());
              else if (sm[2].matched)
                filename = detail::basename(sm[2].str());
            }
            else
            {
              filename = detail::basename(transfer.remote_url().path());
            }

            if (filename.empty() || filename == "." || filename == "/")
              filename = "file";
            local_file_path += filename;
          }

          std::string destination_file_path = local_file_path;


          if (!transfer.overwrite_existing())
          {
            for (std::size_t i = 1; detail::path_exists(destination_file_path); ++i)
            {
              std::stringstream ss;
              ss << detail::directory(local_file_path) << detail::basename_sans_extension(local_file_path) << "_" << i << detail::extension(local_file_path);
              destination_file_path = ss.str();
            }
          }
          else if (detail::is_regular_file(destination_file_path))
          {
            std::remove(destination_file_path.c_str());
          }

          if (std::rename(tmp_file_path.c_str(), destination_file_path.c_str()) != 0)
          {
            std::remove(tmp_file_path.c_str());
            ec = std::error_code(errno, std::system_category());
          }
          else
          {
            std::remove(tmp_file_path.c_str());
          }
        }
      }

      co_return;
    }

    future<void> entity_transfer_client::operator()(file_upload& transfer, std::error_code& ec)
    {
      std::ifstream src_ifs(transfer.local_source(), std::ios::binary);

      if (!src_ifs.good())
      {
        ec = std::error_code(errno, std::system_category());
      }
      else
      {
        std::list<std::pair<std::string, std::string>> headers;
        if (transfer.remote_url().password().size() || transfer.remote_url().username().size())
          headers.emplace_back("authorization", basic_auth(transfer.remote_url().username(), transfer.remote_url().password()));

        auto resp_head = co_await run_transfer("PUT", transfer.remote_url(), ec, transfer.request_headers(), &src_ifs, nullptr, std::bind(&entity_transfer::update_progress, &transfer, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
      }

      co_return;
    }

    future<remote_file_stat::statistics> entity_transfer_client::operator()(remote_file_stat& transfer, std::error_code& ec)
    {
      remote_file_stat::statistics st{};

      std::list<std::pair<std::string, std::string>> headers;
      if (transfer.remote_url().password().size() || transfer.remote_url().username().size())
        headers.emplace_back("authorization", basic_auth(transfer.remote_url().username(), transfer.remote_url().password()));

      auto resp_head = co_await run_transfer("HEAD", transfer.remote_url(), ec, transfer.request_headers(), nullptr, nullptr, std::bind(&entity_transfer::update_progress, &transfer, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

      if (!ec)
      {
        st.file_size = 0;
        if (resp_head.header_exists("content-length"))
          st.file_size = -1;
        else
        {
          std::stringstream ss(resp_head.header("content-length"));
          ss >> st.file_size;
        }

        st.mime_type = resp_head.header("content-type");

        st.modification_date = resp_head.header("last-modified");
      }

      co_return st;
    }

    future<response_head> entity_transfer_client::run_transfer(const std::string& method, uri request_url, std::error_code& ec, std::list<std::pair<std::string,std::string>> header_list, std::istream* req_entity, std::ostream* resp_entity , progress_callback progress)
    {
      response_head ret;
      request_head rh;
      rh.method(method);
      rh.path(request_url.path_with_query());
      for (auto it = header_list.begin(); it != header_list.end(); ++it)
        rh.header(it->first, it->second);

      std::size_t redirect_cnt = 0;
      for ( ; !ec && redirect_cnt <= max_redirects_; ++redirect_cnt)
      {
        auto req = co_await (request_url.scheme_name() == "https" ? secure_client_ : client_ ).make_request(request_url.host(), request_url.port(), rh, ec);
        if (ec)
        {

        }
        else
        {
          if (!req_entity)
          {
            co_await req.end();
          }
          else
          {
            req_entity->clear();
            req_entity->seekg(0, std::ios::beg);

            std::int64_t content_length = -1;
            if (!req.head().header("content-length").empty())
            {
              std::stringstream ss_content_length(req.head().header("content-length"));
              ss_content_length >> content_length;
            }
            std::int64_t total_bytes_sent = 0;

            std::array<char, 8192> buf;

            while (req_entity->good())
            {
              long bytes_in_buf = req_entity->read(buf.data(), buf.size()).gcount();
              if (bytes_in_buf > 0)
              {
                co_await req.send(buf.data(), (std::size_t)bytes_in_buf);
                total_bytes_sent += (std::size_t)bytes_in_buf;
                if (progress)
                  progress(total_bytes_sent, content_length, std::ios::out); //TODO
              }
            }

            co_await req.end();
          }

          auto resp = co_await req.response();



          if (resp.head().has_successful_status())
          {
            std::int64_t content_length = -1;
            if (!resp.head().header("content-length").empty())
            {
              std::stringstream ss_content_length(resp.head().header("content-length"));
              ss_content_length >> content_length;
            }
            std::int64_t total_bytes_received = 0;

            std::array<char, 8192> buf;
            while (resp)
            {
              std::size_t sz = co_await resp.recv(buf.data(), buf.size());
              if (resp_entity)
                resp_entity->write(buf.data(), sz);
              total_bytes_received += sz;
              if (progress)
               progress(total_bytes_received, content_length, std::ios::in); // TODO
            }


            co_return resp.head();
          }
          else if (resp.head().has_redirection_status())
          {
            // TODO: Deal with method changes depending on status code.
            uri redirect_url = resp.head().header("location");
            if (!redirect_url.is_valid() || redirect_cnt == max_redirects_)
            {
              ec = std::make_error_code(status_code_to_errc(resp.head().get_status_code()));
              co_return resp.head();
            }
            else
            {
              if (redirect_url.is_relative())
              {
                redirect_url.host(request_url.host());
                redirect_url.port(request_url.port());
              }

              if (redirect_url.scheme_name().empty())
                redirect_url.scheme_name(request_url.scheme_name());


              std::list<std::pair<std::string,std::string>> redirect_header_list;
              for (auto it = header_list.begin(); it != header_list.end(); ++it)
              {
                if (it->first != "authentication" // TODO: Add option to trust location header
                  && it->first != "cookie") // TODO: Add cookie manager.
                {
                  redirect_header_list.push_back(*it);
                }
              }

              header_list = redirect_header_list;
              request_url = redirect_url;
            }
          }
          else
          {
            ec = std::make_error_code(status_code_to_errc(resp.head().get_status_code()));
            co_return resp.head();
          }
        }
      }

      co_return response_head();
    }
    //================================================================//
  }
}