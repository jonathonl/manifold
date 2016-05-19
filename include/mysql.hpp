
#ifndef MANIFOLD_MYSQL_HPP
#define MANIFOLD_MYSQL_HPP

#include "asio.hpp"

namespace manifold
{
  namespace mysql
  {
    namespace my_c_api
    {
      #include <mysql.h>
    }
  }
}
#include <thread>
#include <memory>
#include <string>
#include <vector>
#include <system_error>
#include <experimental/any>

namespace manifold
{
  namespace mysql
  {
    class session_opts
    {
    public:
      std::string host;
      std::string user;
      std::string password;
      std::string db;
    };

    template <typename Service>
    class basic_session
      : public asio::basic_io_object<Service>
    {
    public:
      explicit basic_session(asio::io_service &io_service, session_opts opts)
        : asio::basic_io_object<Service>(io_service),
        options_(opts),
        my_(nullptr, close_mysql_ptr)
      {
        my_c_api::mysql_init(my_.get());
      }

      template <typename Handler>
      void async_connect(Handler handler)
      {
        this->service.async_connect(options_, handler);
      }

      template <typename Handler>
      void async_query(const std::string& query, Handler handler)
      {
        this->service.async_query(query, handler);
      }

    private:
      session_opts options_;
      std::shared_ptr<my_c_api::MYSQL> my_;

      static void close_mysql_ptr(my_c_api::MYSQL* ptr)
      {
        if (ptr)
          my_c_api::mysql_close(ptr);
        ptr = nullptr;
      }
    };

    class basic_mysql_service
      : public asio::io_service::service
    {
    public:
      basic_mysql_service(asio::io_service& ioservice)
        : asio::io_service::service(ioservice),
        work_(new asio::io_service::work(work_io_service_)),
        work_thread_(std::bind(static_cast<std::size_t (asio::io_service::*)()>(&asio::io_service::run), &work_io_service_))
      {
      }

      ~basic_mysql_service()
      {
        work_.reset();
        //work_io_service_.stop();
        work_thread_.join();  // program is blocked here until the second
        // signal is triggerd
        //work_io_service_.reset();
      }
    private:

      template <typename Handler>
      class connect_operation
      {
      public:
        connect_operation(asio::io_service& ioservice, const std::shared_ptr<my_c_api::MYSQL>& my, const session_opts& opts, Handler handler)
          : io_service_(ioservice),
          options_(opts),
          handler_(handler),
          my_(my)
        {
        }
        void operator()() const
        {
          if (!io_service_.stopped())
          {
            std::error_code ec;

            if (my_c_api::mysql_real_connect(my_.get(), options_.host.c_str(), options_.user.c_str(), options_.password.c_str(), options_.db.c_str(), 0, nullptr, 0) == 0)
              ec = std::error_code((int)std::errc::not_connected, std::generic_category()); // TODO: Get MySQL error.
            this->io_service_.post(std::bind(handler_, ec));
          }
          else
          {
            this->io_service_.post(std::bind(handler_, std::errc::operation_canceled));
          }
        }
      private:
        asio::io_service& io_service_;
        session_opts options_;
        Handler handler_;
        std::shared_ptr<my_c_api::MYSQL> my_;
      };

      template <typename Handler>
      class query_operation
      {
      public:
        query_operation(asio::io_service& ioservice, const std::shared_ptr<my_c_api::MYSQL>& my, const std::string& query, Handler handler)
          : io_service_(ioservice),
          query_(query),
          handler_(handler),
          my_(my)
        {
        }
        void operator()() const
        {
          std::vector<std::map<std::string, std::experimental::any>> assoc_results;
          if (!io_service_.stopped())
          {
            std::error_code ec;


            if (my_c_api::mysql_query(my_.get(), query_.c_str()) != 0)
              ec = std::error_code((int)std::errc::not_connected, std::generic_category()); // TODO: Get MySQL error.
            else
            {
              my_c_api::MYSQL_RES* res = my_c_api::mysql_store_result(my_.get());
              unsigned int num_fields = my_c_api::mysql_num_fields(res);
              my_c_api::MYSQL_ROW row;
              my_c_api::MYSQL_FIELD* field;

              my_c_api::MYSQL_FIELD* fields[num_fields];
              for (unsigned int i = 0; (field = mysql_fetch_field(res)); ++i)
                fields[i] = field;

              unsigned long long num_rows = mysql_num_rows(res);
              assoc_results.reserve(num_rows);

              while ((row = mysql_fetch_row(res)))
              {
                unsigned long* field_lengths = mysql_fetch_lengths(res);
                std::map<std::string, std::experimental::any> tmp_row;
                for (std::size_t i = 0; i < num_fields; ++i)
                {
                  if (row[i] == nullptr)
                  {
                    tmp_row[fields[i]->name];
                  }
                  else if (fields[i]->type == my_c_api::MYSQL_TYPE_STRING || fields[i]->type == my_c_api::MYSQL_TYPE_VAR_STRING)
                  {
                    if (fields[i]->flags & BINARY_FLAG)
                    {
                      std::vector<std::uint8_t> tmp_bin(field_lengths[i]);
                      std::memcpy(tmp_bin.data(), (std::uint8_t*)row[i], field_lengths[i]);
                      tmp_row[fields[i]->name] = std::move(tmp_bin);
                    }
                    else
                    {
                      tmp_row[fields[i]->name] = std::string(row[i]);
                    }
                  }
                  else if (fields[i]->type == my_c_api::MYSQL_TYPE_STRING || fields[i]->type == my_c_api::MYSQL_TYPE_VAR_STRING)
                  {
                  }
                  else
                  {
                    std::stringstream ss_val(row[i]);
                    // NUMBER
                    if (fields[i]->type == my_c_api::MYSQL_TYPE_FLOAT)
                    {
                      float num_val;
                      ss_val >> num_val;
                      tmp_row[fields[i]->name] = num_val;
                    }
                    else if (fields[i]->type == my_c_api::MYSQL_TYPE_DOUBLE)
                    {
                      double num_val;
                      ss_val >> num_val;
                      tmp_row[fields[i]->name] = num_val;
                    }
                    else
                    {
                      // INTEGER
                      if (fields[i]->flags & UNSIGNED_FLAG)
                      {
                        if (fields[i]->type == my_c_api::MYSQL_TYPE_LONGLONG)
                        {
                          std::uint64_t integer_val;
                          ss_val >> integer_val;
                          tmp_row[fields[i]->name] = integer_val;
                        }
                        else if (fields[i]->type == my_c_api::MYSQL_TYPE_LONG)
                        {
                          std::uint32_t integer_val;
                          ss_val >> integer_val;
                          tmp_row[fields[i]->name] = integer_val;
                        }
                        else if (fields[i]->type == my_c_api::MYSQL_TYPE_SHORT)
                        {
                          std::uint16_t integer_val;
                          ss_val >> integer_val;
                          tmp_row[fields[i]->name] = integer_val;
                        }
                        else if (fields[i]->type == my_c_api::MYSQL_TYPE_TINY)
                        {
                          std::uint16_t integer_val; // 16 bit on purpose. Streams treat int8_t's as chars.
                          ss_val >> integer_val;
                          tmp_row[fields[i]->name] = static_cast<std::uint8_t>(integer_val);
                        }
                        else
                        {
                          // TODO: Unsupported type
                        }
                      }
                      else
                      {
                        if (fields[i]->type == my_c_api::MYSQL_TYPE_LONGLONG)
                        {
                          std::int64_t integer_val;
                          ss_val >> integer_val;
                          tmp_row[fields[i]->name] = integer_val;
                        }
                        else if (fields[i]->type == my_c_api::MYSQL_TYPE_LONG)
                        {
                          std::int32_t integer_val;
                          ss_val >> integer_val;
                          tmp_row[fields[i]->name] = integer_val;
                        }
                        else if (fields[i]->type == my_c_api::MYSQL_TYPE_SHORT)
                        {
                          std::int16_t integer_val;
                          ss_val >> integer_val;
                          tmp_row[fields[i]->name] = integer_val;
                        }
                        else if (fields[i]->type == my_c_api::MYSQL_TYPE_TINY)
                        {
                          std::int16_t integer_val; // 16 bit on purpose. Streams treat int8_t's as chars.
                          ss_val >> integer_val;
                          tmp_row[fields[i]->name] = static_cast<std::int8_t>(integer_val);
                        }
                        else
                        {
                          // TODO: Unsupported type.
                        }
                      }
                    }
                  }
                }


                assoc_results.push_back(std::move(tmp_row));
              }
            }
            this->io_service_.post(std::bind(handler_, ec));
          }
          else
          {
            this->io_service_.post(std::bind(handler_, std::errc::operation_canceled));
          }
        }
      private:
        asio::io_service& io_service_;
        Handler handler_;
        std::shared_ptr<my_c_api::MYSQL> my_;
        std::string query_;
      };

    public:
      template <typename Handler>
      void async_connect(const std::shared_ptr<my_c_api::MYSQL>& my, const session_opts& opts, Handler handler)
      {
        this->work_io_service_.post(connect_operation<Handler>(this->get_io_service(), my, opts, handler));
      }

      template <typename Handler>
      void async_query(const std::shared_ptr<my_c_api::MYSQL>& my, const std::string& query, Handler handler)
      {
        this->work_io_service_.post(query_operation<Handler>(this->get_io_service(), my, query, handler));
      }

    private:
      std::unique_ptr<asio::io_service::work> work_;
      asio::io_service work_io_service_;
      std::thread work_thread_;
    };

    typedef basic_session<basic_mysql_service> session;
  }
}

#endif //MANIFOLD_MYSQL_HPP