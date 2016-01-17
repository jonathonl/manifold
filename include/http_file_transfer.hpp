#pragma once

#ifndef MANIFOLD_HTTP_FILE_TRANSFER_HPP
#define MANIFOLD_HTTP_FILE_TRANSFER_HPP

#include "uniform_resource_identifier.hpp"
#include "http_client.hpp"
#include "http_server.hpp"

#include <regex>
#include <fstream>
#include <random>

namespace manifold
{
  namespace http
  {
    class document_root
    {
    public:
      document_root(const std::string& path, const std::map<std::string, std::string>& user_credentials = {});
      ~document_root();
      void operator()(server::request&& req, server::response&& res, const std::smatch& matches);
    private:
      std::string path_to_root_;
      std::map<std::string, std::string> user_credentials_;
      std::minstd_rand rng_;

      void handle_head(server::response&& res, const std::string& file_path);
      void handle_get(server::response&& res, const std::string& file_path);
      void handle_put(server::request&& req, server::response&& res, const std::string& file_path);
    };

    class file_transfer_error
    {
    public:
      file_transfer_error() {}
      file_transfer_error(const std::string& msg)
        : message_(msg)
      {
      }
      virtual ~file_transfer_error() {}

      operator bool() const { return (message_.size() > 0); }
      const std::string& message() const { return this->message_; }
    private:
      std::string message_;
    };

    class file_upload
    {
    };

    class file_download
    {
    public:
      file_download(asio::io_service& ioservice, const uri& remote_source, const std::string& local_destination, bool replace_existing_file = false);
      ~file_download();

      void on_complete(std::function<void(const file_transfer_error& err, const std::string& local_file_path)>&& cb);
      void on_complete(const std::function<void(const file_transfer_error& err, const std::string& local_file_path)>& cb);
      void on_progress(std::function<void(std::uint64_t transfered, std::uint64_t total)>&& cb);
      void on_progress(const std::function<void(std::uint64_t transfered, std::uint64_t total)>& cb);
      void cancel();
    private:
      std::unique_ptr<client> c_;
      std::ofstream file_;
      const uri url_;
      const std::string local_path_;
      const bool replace_existing_file_;

      void on_close_handler(errc ec);
      std::function<void(std::uint64_t transfered, std::uint64_t total)> on_progress_;
      std::function<void(const file_transfer_error& err, const std::string& local_file_path)> on_complete_;
      bool completed_ = false;
      file_transfer_error err_;
      std::string result_;
    };

    class file_check
    {
    };
  }
}

#endif //MANIFOLD_HTTP_FILE_TRANSFER_HPP