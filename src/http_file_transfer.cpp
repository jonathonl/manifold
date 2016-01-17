
#include "http_file_transfer.hpp"

#include <chrono>

namespace manifold
{
  namespace http
  {
    //================================================================//
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
        ret = "." +  tmp.substr(0, tmp.find_last_of("."));
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
    //================================================================//

    //================================================================//
    document_root::document_root(const std::string& path, const std::map<std::string, std::string>& user_credentials)
      : path_to_root_(path), user_credentials_(user_credentials)
    {
      std::seed_seq seed = {(long)(this), (long)std::chrono::high_resolution_clock::now().time_since_epoch().count()};
      this->rng_.seed(seed);
    }

    document_root::~document_root()
    {
    }

    void document_root::operator()(server::request&& req, server::response&& res, const std::smatch& matches)
    {
      res.head().header("content-type", "text/plain");

      if (matches.size() < 2)
      {
        // TODO: Handle invalid regex.
      }
      else
      {
        bool allowed = true;

        if (!allowed)
        {
          // TODO: Check authorization header
        }
        else
        {
          std::string file_path = this->path_to_root_ + matches[2].str();

          if (req.head().method() == "HEAD")
          {
            this->handle_head(std::move(res), file_path);
          }
          else if (req.head().method() == "GET")
          {
            this->handle_get(std::move(res), file_path);
          }
          else if (req.head().method() == "PUT")
          {
            this->handle_put(std::move(req), std::move(res), file_path);
          }
          else
          {
            res.head().status_code(status_code::method_not_allowed);
            res.end(status_code_to_reason_phrase(res.head().status_code()));
          }
        }
      }
    }

    void document_root::handle_head(server::response&& res, const std::string& file_path)
    {
      struct stat st;
      if (stat(file_path.c_str(), &st) != 0 || (st.st_mode & S_IFREG) == 0)
      {
        res.head().status_code(status_code::not_found);
        res.end();
      }
      else
      {
        res.head().header("content-length", std::to_string(st.st_size));
        res.end();
      }
    }

    void document_root::handle_get(server::response&& res, const std::string& file_path)
    {
      struct stat st;
      if (stat(file_path.c_str(), &st) != 0 || (st.st_mode & S_IFREG) == 0)
      {
        res.head().status_code(status_code::not_found);
        res.end(status_code_to_reason_phrase(res.head().status_code()));
      }
      else
      {
        auto ifs = std::make_shared<std::ifstream>(file_path, std::ios::binary);
        if (!ifs->good())
        {
          res.head().status_code(status_code::internal_server_error);
          res.end(status_code_to_reason_phrase(res.head().status_code()));
        }
        else
        {
          auto res_ptr = std::make_shared<server::response>(std::move(res));
          res_ptr->head().header("content-length", std::to_string(st.st_size));

          std::array<char, 4096> buf;
          long bytes_in_buf = ifs->read(buf.data(), buf.size()).gcount();
          if (!ifs->good())
          {
            if (bytes_in_buf > 0)
              res_ptr->end(buf.data(), (std::size_t)bytes_in_buf);
            else
              res_ptr->end();
          }
          else
          {
            res_ptr->on_drain([ifs, res_ptr]()
            {
              std::array<char, 4096> buf;
              long bytes_in_buf = ifs->read(buf.data(), buf.size()).gcount();
              if (bytes_in_buf > 0)
                res_ptr->send(buf.data(), (std::size_t)bytes_in_buf);

              if (!ifs->good())
                res_ptr->end();
            });
            res_ptr->send(buf.data(), (std::size_t)bytes_in_buf);
          }
        }
      }
    }

    void document_root::handle_put(server::request&& req, server::response&& res, const std::string& file_path)
    {
      std::stringstream ss;
      ss << file_path << "_" << std::to_string(this->rng_()) << ".tmp";
      std::string tmp_file_path(ss.str());
      auto ofs = std::make_shared<std::ofstream>(tmp_file_path, std::ios::binary);

      if (!ofs->good())
      {
        res.head().status_code(status_code::internal_server_error);
        res.end(status_code_to_reason_phrase(res.head().status_code()));
      }
      else
      {
        // TODO: send continue if expected.

        auto res_ptr = std::make_shared<server::response>(std::move(res));


        req.on_data([ofs](const char*const data, std::size_t data_sz)
        {
          ofs->write(data, data_sz);
        });

        req.on_end([res_ptr, ofs, tmp_file_path, file_path]()
        {
          if (!ofs->good())
          {
            ofs->close();
            std::remove(tmp_file_path.c_str());
            res_ptr->head().status_code(status_code::internal_server_error);
            res_ptr->end(status_code_to_reason_phrase(res_ptr->head().status_code()));
          }
          else
          {
            ofs->close();
            if (std::rename(tmp_file_path.c_str(), file_path.c_str()) != 0)
            {
              std::remove(tmp_file_path.c_str());
              res_ptr->head().status_code(status_code::internal_server_error);
              res_ptr->end(status_code_to_reason_phrase(res_ptr->head().status_code()));
            }
            else
            {
              res_ptr->end();
            }
          }
        });

        req.on_close([tmp_file_path, ofs](errc ec)
        {
          ofs->close();
          std::remove(tmp_file_path.c_str());
        });
      }
    }
    //================================================================//

    //================================================================//
    file_download::file_download(asio::io_service &ioservice, const uri& remote_source, const std::string& local_destination, bool replace_existing_file)
      : url_(remote_source), local_path_(local_destination), replace_existing_file_(replace_existing_file)
    {
      std::string path = "/file.txt";
      auto b = basename(path);
      auto bse = basename_sans_extension(path);
      auto e = extension(path);
      auto d = directory(path);
      auto i = 1;

      if (remote_source.scheme_name() == "https")
        c_ = std::unique_ptr<client>(new client(ioservice, remote_source.host(), client::ssl_options(), remote_source.port()));
      else
        c_ = std::unique_ptr<client>(new client(ioservice, remote_source.host(), remote_source.port()));

      c_->on_connect([this]()
      {
        auto req = this->c_->make_request();

        req.on_response([this](client::response&& res)
        {
          if (!res.head().has_successful_status())
          {
            // TODO: be more specific.
            this->err_ = file_transfer_error("HTTP Error (" + http::status_code_to_reason_phrase(res.head().status_code()) + ")");
          }
          else
          {
            std::string local_file_path = this->local_path_;
            std::replace(local_file_path.begin(), local_file_path.end(), '\\', '/');
            if (is_directory(local_file_path))
            {
              if (local_file_path.size() && local_file_path.back() != '/')
                local_file_path += "/";
              std::string content_disposition = res.head().header("content-disposition");
              std::string filename;
              std::regex exp(".*filename=(?:\"([^\"]*)\"|([^\\s;]*)).*", std::regex::ECMAScript);
              std::smatch sm;
              if (std::regex_match(content_disposition, sm, exp))
              {
                if (sm[1].matched)
                  filename = basename(sm[1].str());
                else if (sm[2].matched)
                  filename = basename(sm[2].str());
              }
              else
              {
                filename = basename(this->url_.path());
              }

              if (filename.empty() || filename == "." || filename == "/")
                filename = "file";
              local_file_path += filename;
            }

            std::string destination_file_path = local_file_path;



            if(!this->replace_existing_file_)
            {
              for(std::size_t i = 1; path_exists(destination_file_path); ++i)
              {
                std::stringstream ss;
                ss << directory(local_file_path) << basename_sans_extension(local_file_path) << "_" << i << extension(local_file_path);
                destination_file_path = ss.str();
              }
            }
            else if(is_regular_file(destination_file_path))
            {
              std::remove(destination_file_path.c_str());
            }

            this->file_.open(destination_file_path, std::ios::binary);

            if (!this->file_.good())
            {
              this->err_ = file_transfer_error("Could Not Open File For Writing");
              res.close(http::errc::cancel);
            }
            else
            {
              this->result_ = destination_file_path;
              res.on_data([this](const char*const data, std::size_t data_sz)
              {
                this->file_.write(data, data_sz);
              });

//              res.on_end([]()
//              {
//              });
            }
          }
        });

        req.on_close(std::bind(&file_download::on_close_handler, this, std::placeholders::_1));

        req.head().path(this->url_.path_with_query());
        req.end();
      });

      this->c_->on_close(std::bind(&file_download::on_close_handler, this, std::placeholders::_1));
    }

    file_download::~file_download()
    {
      this->cancel();
      assert(!this->file_.is_open());
    }

    void file_download::on_close_handler(errc ec)
    {
      if (!this->completed_)
      {
        this->completed_ = true;

        this->file_.close();

        if (ec != errc::no_error)
        {
          if (this->result_.size())
            std::remove(this->result_.c_str());

          if (!err_)
          {
            this->err_ = file_transfer_error("Connection Closed Prematurely (" + std::to_string((unsigned) ec) + ")");
          }
        }

        this->on_complete_ ? this->on_complete_(this->err_, this->result_) : void();
        this->on_complete_ = nullptr;
        this->c_->close();
      }
    }

    void file_download::on_complete(std::function<void(const file_transfer_error& err, const std::string& local_file_path)>&& cb)
    {
      if (this->completed_)
        cb(err_, result_);
      else
        on_complete_ = std::move(cb);
    }

    void file_download::on_complete(const std::function<void(const file_transfer_error &err, const std::string &local_file_path)>& cb)
    {
      this->on_complete(std::function<void(const file_transfer_error &err, const std::string &local_file_path)>(cb));
    }

    void file_download::on_progress(std::function<void(std::uint64_t transfered, std::uint64_t total)> &&cb)
    {
      on_progress_ = std::move(cb);
    }

    void file_download::on_progress(const std::function<void(std::uint64_t transfered, std::uint64_t total)> &cb)
    {
      this->on_progress(std::function<void(std::uint64_t transfered, std::uint64_t total)>(cb));
    }

    void file_download::cancel()
    {
      this->c_->close();
    }
    //================================================================//
  }
}