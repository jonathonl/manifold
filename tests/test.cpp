
#include <memory>
#include <fstream>
#include <cstdio>
#include <iomanip>
#include <tuple>
#include <thread>
#include <iostream>

#include "asio.hpp"
#include "manifold/http_server.hpp"
#include "manifold/http_client.hpp"
//#include "manifold/http_client.hpp"
#include "manifold/http_router.hpp"
#include "manifold/hpack.hpp"
#include "manifold/http_file_transfer.hpp"

//#include "mysql.hpp"
#include <experimental/any>



using namespace manifold;

//================================================================//
//class move_only
//{
//public:
//  move_only() { }
//  move_only(move_only&& source) { }
//  move_only& operator=(move_only&& source) { return *this; }
//private:
//  move_only(const move_only&) = delete;
//  move_only& operator=(const move_only&) = delete;
//};
//
//std::tuple<move_only, move_only, int> return_move_only()
//{
//  static move_only ret;
//  static move_only ret2;
//  return std::make_tuple(std::move(ret), std::move(ret2), 5);
//}
//
//void mysql_test()
//{
//  move_only first;
//  move_only second;
//  int third;
//  std::tie(first, second, third) = return_move_only();
//
//  std::vector<std::string> v = {"foobar","fooman"};
//  std::string s = "hello world";
//  std::map<std::string, std::string> m;
//  m["hello"] = "world";
//
//  mysql::session_opts my_ops;
//  my_ops.host = "localhost";
//  my_ops.db = "gasp";
//  my_ops.user = "gasp_user";
//  my_ops.password = "foobar";
//  mysql::session my(ioservice, my_ops);
//
//
//
//  my.async_connect([&my](const std::error_code& ec)
//  {
//    if (ec)
//    {
//      std::cout << ec.message() << std::endl;
//    }
//    else
//    {
//      my.async_query("SELECT * FROM jobs", [](const std::error_code& ec, const std::vector<std::map<std::string, std::experimental::any>>& res)
//      {
//
//        if (ec)
//        {
//          std::cout << ec.message() << std::endl;
//        }
//        else
//        {
//          for (std::size_t i = 0; i<res.size(); ++i)
//          {
//            const std::map<std::string, std::experimental::any>& r = res[i];
//            std::cout
//            << any_cast<const std::string&>(res[i].at("id")) << std::endl
//            << any_cast<unsigned int>(res[i].at("user_id")) << std::endl
//            << any_cast<std::string>(res[i].at("name")) << std::endl
//            << any_cast<std::string>(res[i].at("error_message")) << std::endl
//            << any_cast<unsigned int>(res[i].at("status_id")) << std::endl
//            << any_cast<std::string>(res[i].at("creation_date")) << std::endl
//            << any_cast<std::string>(res[i].at("modified_date")) << std::endl << std::endl;
//          }
//        }
//      });
//    }
//  });
//
//  ioservice.run();
//}
//================================================================//

//================================================================//
void run_hpack_test()
{
  std::size_t http2_default_table_size = 4096;
  hpack::encoder enc(http2_default_table_size);
  hpack::decoder dec(http2_default_table_size);

  std::list<hpack::header_field> send_headers{
    {":path","/"},
    {":method","GET"},
    {"content-type","application/json; charset=utf8"},
    {"content-length","30"},
    {"custom-header","foobar; baz"},
    {"custom-header2","NOT INDEXED", hpack::cacheability::no}};
  std::list<hpack::header_field> send_headers2{
    {":path","/"},
    {":method","GET"},
    {"custom-header","foobar; baz3"},
    {"custom-header2","NOT INDEXED", hpack::cacheability::never}};

  std::list<hpack::header_field> recv_headers;
  std::list<hpack::header_field> recv_headers2;

  std::string serialized_headers;
  enc.encode(send_headers, serialized_headers);
  dec.decode(serialized_headers.begin(), serialized_headers.end(), recv_headers);

  for (auto it : recv_headers)
    std::cout << it.name << ": " << it.value << std::endl;
  std::cout << std::endl;

  // Encoders can use table size updates to clear dynamic table.
  enc.add_table_size_update(0);
  enc.add_table_size_update(4096);

  serialized_headers = "";
  enc.encode(send_headers2, serialized_headers);
  dec.decode(serialized_headers.begin(), serialized_headers.end(), recv_headers2);
  for (auto it : recv_headers2)
    std::cout << it.name << ": " << it.value << std::endl;
}
//================================================================//

////================================================================//
//void handle_push_promise(http::client::request && req, std::uint32_t dependency_stream_id)
//{
//
//  req.on_response([](http::client::response && resp)
//  {
//    for (auto it : resp.head().raw_headers())
//      std::cout << it.first << ": " << it.second << std::endl;
//
//    resp.on_data([](const char*const d, std::size_t sz)
//    {
//      std::cout << std::string(d, sz) << std::endl;
//    });
//  });
//}
////================================================================//

////================================================================//
//void start_server(asio::io_service& ioservice)
//{
//  http::router app;
//
//  http::document_root get_doc_root("./");
//  get_doc_root.add_credentials("user", "pass");
//  app.register_handler(std::regex("^/files/(.*)$"), "HEAD", http::document_root("./"));
//  app.register_handler(std::regex("^/files/(.*)$"), "GET", std::ref(get_doc_root));
//  app.register_handler(std::regex("^/files/(.*)$"), "PUT", http::document_root("./"));
//  get_doc_root.add_credentials("user", "password");
//
//  app.register_handler(std::regex("^/redirect-url$"), [](http::server::request&& req, http::server::response&& res, const std::smatch& matches)
//  {
//    res.head().set_status_code(http::status_code::found);
//    res.head().header("location","/new-url");
//    res.end();
//  });
//
//  app.register_handler(std::regex("^/(.*)$"), [](http::server::request&& req, http::server::response&& res, const std::smatch& matches)
//  {
//    auto res_ptr = std::make_shared<http::server::response>(std::move(res));
//
//    for (auto it : req.head().raw_headers())
//      std::cout << it.first << ": " << it.second << std::endl;
//
//    auto req_entity = std::make_shared<std::stringstream>();
//    req.on_data([req_entity](const char*const data, std::size_t datasz)
//    {
//      req_entity->write(data, datasz);
//    });
//
//    req.on_end([res_ptr, req_entity]()
//    {
//      auto push_promise = res_ptr->send_push_promise(http::request_head("/push-url"));
//
//      res_ptr->send("Received: " + req_entity->str() + "\n");
//      res_ptr->end();
//
//      push_promise.fulfill([](http::server::request&& rq, http::server::response&& rs)
//      {
//        // TODO: have on_end immidiately callback if closed or half closed remote.
//        rs.end("Here's the promised data.");
//      });
//
//    });
//
//    req.on_close([](const std::error_code& e)
//    {
//      std::cout << "on_close called on server" << std::endl;
//    });
//
//  });
//
//
//  std::vector<char> chain;
//  std::vector<char> key;
//  std::vector<char> dhparam;
//  {
//    std::ifstream ifs("tests/certs2/cert.crt");
//    if (ifs.good())
//      chain.assign(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());
//  }
//  {
//    std::ifstream ifs("tests/certs2/cert.key");
//    if (ifs.good())
//      key.assign(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());
//  }
//  {
//    std::ifstream ifs("tests/certs/dh2048.pem");
//    if (ifs.good())
//      dhparam.assign(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());
//  }
//
//  asio::ssl::context server_ssl_ctx(asio::ssl::context::tlsv12);
//  server_ssl_ctx.use_certificate_chain(asio::buffer(chain.data(), chain.size()));
//  server_ssl_ctx.use_private_key(asio::buffer(key.data(), key.size()), asio::ssl::context::pem);
//  //server_ssl_ctx.use_tmp_dh(asio::buffer(dhparam.data(), dhparam.size()));
//
//  http::server srv(ioservice, server_ssl_ctx, 8080);
//  srv.reset_timeout(std::chrono::seconds(15));
//  srv.listen(std::bind(&http::router::route, &app, std::placeholders::_1, std::placeholders::_2));
//  ioservice.run();
//}
////================================================================//

////================================================================//
//void client_to_local_server_test(asio::io_service& ioservice)
//{
//  asio::ssl::context client_ssl_ctx(asio::ssl::context::tlsv12);
//  http::client agnt(ioservice, client_ssl_ctx);
//  http::stream_client stream_agnt(agnt);
//  http::file_transfer_client file_transfer_agnt(stream_agnt);
//
//  agnt.reset_timeout(std::chrono::seconds(5));
//
//  http::file_transfer_client::options ops;
//  ops.replace_existing_file = true;
//  auto t = std::chrono::system_clock::now().time_since_epoch();
//  std::cout << "Starting Download ..." << std::endl;
//  auto last_percent = std::make_shared<std::uint64_t>(0);
//
//  {
//    auto p = file_transfer_agnt.download_file(uri("https://user:password@localhost:8080/files/test_cmp.rfcmp"), "./")
//      .on_progress([last_percent](std::uint64_t transferred, std::uint64_t total)
//      {
//        if (total)
//        {
//          std::uint64_t percent = (static_cast<double>(transferred) / static_cast<double>(total)) * 100;
//          if (percent > *last_percent)
//          {
//            *last_percent = percent;
//            std::cout << "\r" << percent << "%" << std::flush;
//          }
//        }
//      }).on_complete([t](const std::error_code& ec, const std::string& file_path)
//      {
//        std::cout << std::endl;
//        if (ec)
//        {
//          std::cout << ec.message() << std::endl;
//        }
//        else
//        {
//          std::cout << "DL SUCCEEDED" << std::endl;
//          std::cout << "SECONDS: " << std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch() - t).count() << std::endl;
//        }
//      });
//  }
//
//
//
//  for (size_t i = 0; i < 3; ++i)
//  {
//    file_transfer_agnt.download_file(uri("https://user:password@localhost:8080/files/readme.md"), "./").on_complete([i](const std::error_code& ec, const std::string& file_path)
//    {
//      if (ec)
//      {
//        std::cout << ec.message() << std::endl;
//      }
//      else
//      {
//        std::cout << "GET " << std::setfill('0') << std::setw(2) << i;
//        std::cout << " SUCCEEDED" << std::endl;
//      }
//    });
//  }
//
//  ioservice.run();
//}
////================================================================//


//using std::experimental::any_cast;
//using std::experimental::any;
using re = std::regex;
//################################################################//
int main()
{
  http::router app;

  http::document_root get_doc_root("./");
  get_doc_root.add_credentials("user", "pass");
  app.register_handler(re("^/files/(.*)$"), "HEAD", http::document_root("./"));
  app.register_handler(re("^/files/(.*)$"), "GET", std::ref(get_doc_root));
  app.register_handler(re("^/files/(.*)$"), "PUT", http::document_root("./"));
  get_doc_root.add_credentials("user", "password");

  app.register_handler(re("^/redirect-url$"), [](http::server::request req, http::server::response res, std::smatch matches) -> manifold::future<void>
  {
    res.head().set_status_code(http::status_code::found);
    res.head().header("location","/new-url");
    co_await res.end();
    co_return;
  });

  app.register_handler(re("^/(.*)$"), [](http::server::request req, http::server::response res, std::smatch matches) -> manifold::future<void>
  {
    std::cout << (req.head().method() + " " + req.head().path()) << std::endl;

    std::cout << "matches size: " << matches.size() << std::endl;
    for (auto it = matches.begin(); it != matches.end(); ++it)
    {
      std::cout << "match: " << it->str() << std::endl;
    }

    std::string buf(1024, '\0');

    while (req)
    {
      std::size_t amount = co_await req.recv(&buf[0], buf.size());
      buf.resize(amount);
      std::cout << buf << std::endl;
    }

    res.head().set_status_code(200);
    for (std::size_t i = 0; i < 10 && res; ++i)
    {
      co_await res.send("i: " + std::to_string(i) + "\n"); //s.data(), s.size());
    }

    co_await res.end(buf);


    co_return;
  });

  asio::io_service ioservice;

  if (false)
  {
    asio::ssl::context client_ssl_ctx(asio::ssl::context::tlsv12);
    manifold::http::client client(ioservice, client_ssl_ctx);

    auto req_fn = [](manifold::http::client& client) -> manifold::future<void>
    {
      std::error_code ec;
      http::request_head rh;

      http::client::request req = co_await client.make_request({http::endpoint::secure, "www.google.com", 443}, rh, ec);
      if (ec)
      {
        std::cerr << ec.message() << std::endl;
      }
      else
      {
        http::client::response res = co_await req.response();

        std::array<char, 1024> buf;
        while (res)
        {
          std::size_t amount = co_await res.recv(buf.data(), buf.size());
          std::cout << std::string(buf.data(), amount);

        }
      }
    };

    req_fn(client);

    ioservice.run();
  }

  std::vector<char> chain;
  std::vector<char> key;
  std::vector<char> dhparam;
  {
    std::ifstream ifs("tests/certs2/cert.crt");
    if (ifs.good())
      chain.assign(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());
  }
  {
    std::ifstream ifs("tests/certs2/cert.key");
    if (ifs.good())
      key.assign(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());
  }
  {
    std::ifstream ifs("tests/certs/dh2048.pem");
    if (ifs.good())
      dhparam.assign(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());
  }

  asio::ssl::context server_ssl_ctx(asio::ssl::context::tlsv12);
  server_ssl_ctx.use_certificate_chain(asio::buffer(chain.data(), chain.size()));
  server_ssl_ctx.use_private_key(asio::buffer(key.data(), key.size()), asio::ssl::context::pem);
  //server_ssl_ctx.use_tmp_dh(asio::buffer(dhparam.data(), dhparam.size()));

  http::server srv(ioservice, server_ssl_ctx, 8080);
  srv.reset_timeout(std::chrono::seconds(15));
  srv.listen(std::bind(&http::router::route, &app, std::placeholders::_1, std::placeholders::_2));
//  srv.listen([](http::server::request req, http::server::response res) -> manifold::future<void>
//  {
//    std::cout << (req.head().method() + " " + req.head().path()) << std::endl;
//
//    std::string buf(1024, '\0');
//
//    while (req)
//    {
//      std::size_t amount = co_await req.recv(&buf[0], buf.size());
//      buf.resize(amount);
//      std::cout << buf << std::endl;
//    }
//
//    res.head().set_status_code(200);
//    for (std::size_t i = 0; i < 10 && res; ++i)
//    {
//      co_await res.send("i: " + std::to_string(i) + "\n"); //s.data(), s.size());
//    }
//
//    co_await res.end(buf);
//
//
//    co_return;
//  });

  asio::ssl::context client_ssl_ctx(asio::ssl::context::tlsv12);
  manifold::http::client client(ioservice, client_ssl_ctx);

  auto req_fn = [](manifold::http::client& client) -> manifold::future<void>
  {
    std::error_code ec;
    http::request_head rh;

    http::client::request req = co_await client.make_request({http::endpoint::secure, "localhost", 8080}, rh, ec);
    if (ec)
    {
      std::cerr << ec.message() << std::endl;
    }
    else
    {
      http::client::response res = co_await req.response();

      std::array<char, 1024> buf;
      while (res)
      {
        std::size_t amount = co_await res.recv(buf.data(), buf.size());

      }
    }
  };

  req_fn(client);

  ioservice.run();


  return 0;
}
//################################################################//
