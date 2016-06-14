
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

    //template <typename Service>
    class basic_session
    {
    private:
      template <typename Handler>
      class connect_operation
      {
      public:
        connect_operation(asio::io_service& ioservice, const std::shared_ptr<my_c_api::MYSQL>& my, const session_opts& opts, Handler handler)
          : work_(ioservice),
          options_(opts),
          handler_(handler),
          my_(my)
        {
        }
        void operator()()
        {
          std::error_code ec;
          if (!work_.get_io_service().stopped())
          {


            if (my_c_api::mysql_real_connect(my_.get(), options_.host.c_str(), options_.user.c_str(), options_.password.c_str(), options_.db.c_str(), 0, nullptr, 0) == 0)
              ec = std::error_code((int)std::errc::not_connected, std::generic_category()); // TODO: Get MySQL error.
            this->work_.get_io_service().post(std::bind(handler_, ec));
          }
          else
          {
            ec = std::error_code((int)std::errc::operation_canceled, std::generic_category());
            this->work_.get_io_service().post(std::bind(handler_, ec));
          }
        }
      private:
        asio::io_service::work work_;
        session_opts options_;
        Handler handler_;
        std::shared_ptr<my_c_api::MYSQL> my_;
      };

      template <typename Handler>
      class query_operation
      {
      public:
        query_operation(asio::io_service& ioservice, const std::shared_ptr<my_c_api::MYSQL>& my, const std::string& query, Handler handler)
          : work_(ioservice),
          query_(query),
          handler_(handler),
          my_(my)
        {
        }
        void operator()()
        {
          std::vector<std::map<std::string, std::experimental::any>> assoc_results;
          if (!work_.get_io_service().stopped())
          {
            std::error_code ec;


            if (my_c_api::mysql_query(my_.get(), query_.c_str()) != 0)
            {
              std::cout << my_c_api::mysql_error(my_.get()) << std::endl;
              ec = std::error_code((int) std::errc::not_connected, std::generic_category()); // TODO: Get MySQL error.
            }
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
            this->work_.get_io_service().post(std::bind(handler_, ec, assoc_results));
          }
          else
          {
            this->work_.get_io_service().post(std::bind(handler_, std::error_code((int)std::errc::operation_canceled, std::generic_category()), assoc_results));
          }
        }
      private:
        asio::io_service::work work_;
        std::string query_;
        Handler handler_;
        std::shared_ptr<my_c_api::MYSQL> my_;
      };
    public:
      explicit basic_session(asio::io_service& ioservice, session_opts opts)
        : io_service_(ioservice),
          options_(opts)
      {
        my_c_api::MYSQL* tmp = my_c_api::mysql_init(NULL);
        if (tmp)
          my_ = std::shared_ptr<my_c_api::MYSQL>(tmp, close_mysql_ptr);
      }

      template <typename Handler>
      void async_connect(Handler handler)
      {
        asio::post(asio::system_executor(), connect_operation<Handler>(io_service_, my_, options_, handler));
      }

      template <typename Handler>
      void async_query(const std::string& query, Handler handler)
      {
        asio::post(asio::system_executor(), query_operation<Handler>(io_service_, my_, query, handler));
      }



    private:
      asio::io_service& io_service_;
      session_opts options_;
      std::shared_ptr<my_c_api::MYSQL> my_;

      static void close_mysql_ptr(my_c_api::MYSQL* ptr)
      {
        if (ptr)
          my_c_api::mysql_close(ptr);
        ptr = nullptr;
      }
    };

    typedef basic_session session;
  }
}

#endif //MANIFOLD_MYSQL_HPP