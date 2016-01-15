
#include "http_file_transfer.hpp"

#include <regex>

namespace manifold
{
  namespace http
  {
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

  }
}