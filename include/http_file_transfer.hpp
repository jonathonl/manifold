#pragma once

#ifndef MANIFOLD_HTTP_FILE_TRANSFER_HPP
#define MANIFOLD_HTTP_FILE_TRANSFER_HPP

#include "uniform_resource_identifier.hpp"
#include "http_client.hpp"

#include <fstream>

namespace manifold
{
  namespace http
  {
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