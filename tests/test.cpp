
#include <memory>
#include <fstream>
#include <cstdio>
#include <iomanip>
#include <tuple>
#include <thread>

#include "asio.hpp"
#include "http_server.hpp"
#include "http_client.hpp"
#include "http_router.hpp"
#include "hpack.hpp"



using namespace manifold;

class move_only
{
public:
  move_only() { }
  move_only(move_only&& source) { }
  move_only& operator=(move_only&& source) { return *this; }
private:
  move_only(const move_only&) = delete;
  move_only& operator=(const move_only&) = delete;
};

std::tuple<move_only, move_only, int> return_move_only()
{
  static move_only ret;
  static move_only ret2;
  return std::make_tuple(std::move(ret), std::move(ret2), 5);
}
//================================================================//
void handle_push_promise(http::client::request && req, std::uint32_t dependency_stream_id)
{

  req.on_response([](http::client::response && resp)
  {
    for (auto it : resp.head().raw_headers())
      std::cout << it.first << ": " << it.second << std::endl;

    resp.on_data([](const char*const d, std::size_t sz)
    {
      std::cout << std::string(d, sz) << std::endl;
    });
  });
}
//================================================================//

//################################################################//
int main()
{
  move_only first;
  move_only second;
  int third;
  std::tie(first, second, third) = return_move_only();

  asio::io_service ioservice;

//  //----------------------------------------------------------------//
//  std::uint32_t plus_sign_code = (0x7fb << (32 - 11));
//  auto res = hpack::huffman_code_tree.find(hpack::huffman_code(plus_sign_code, 32));
//  if (res != hpack::huffman_code_tree.end())
//    std::cout << res->second << std::endl;
//
//  std::string compressed_literal = {(char)0xf1,(char)0xe3,(char)0xc2,(char)0xe5,(char)0xf2,(char)0x3a,(char)0x6b,(char)0xa0,(char)0xab,(char)0x90,(char)0xf4,(char)0xff};
//  for (auto it = compressed_literal.begin(); it != compressed_literal.end(); ++it)
//    std::cout << std::hex << (unsigned int)(std::uint8_t)(*it) << std::dec << std::endl;
//  std::string uncompressed_literal;
//  hpack::decoder::huffman_decode(compressed_literal.begin(), compressed_literal.end(), uncompressed_literal);
//  std::cout << "uncompressed size: " << uncompressed_literal.size() << std::endl;
//  std::cout << "uncompressed value: " << uncompressed_literal << std::endl;
//
////  for (auto it = hpack::huffman_code_tree.begin(); it != hpack::huffman_code_tree.end(); ++it)
////    std::cout << "- " << std::hex << it->first.msb_code << std::dec << " | " << it->second << std::endl;
//
//  std::cout.flush();
//
//  //----------------------------------------------------------------//
//
//  //----------------------------------------------------------------//
//  // HPack Test
//  //
//  std::size_t http2_default_table_size = 4096;
//  hpack::encoder enc(http2_default_table_size);
//  hpack::decoder dec(http2_default_table_size);
//
//  std::list<hpack::header_field> send_headers{
//    {":path","/"},
//    {":method","GET"},
//    {"content-type","application/json; charset=utf8"},
//    {"content-length","30"},
//    {"custom-header","foobar; baz"},
//    {"custom-header2","NOT INDEXED", hpack::cacheability::no}};
//  std::list<hpack::header_field> send_headers2{
//    {":path","/"},
//    {":method","GET"},
//    {"custom-header","foobar; baz3"},
//    {"custom-header2","NOT INDEXED", hpack::cacheability::never}};
//
//  std::list<hpack::header_field> recv_headers;
//  std::list<hpack::header_field> recv_headers2;
//
//  std::string serialized_headers;
//  enc.encode(send_headers, serialized_headers);
//  dec.decode(serialized_headers.begin(), serialized_headers.end(), recv_headers);
//
//  for (auto it : recv_headers)
//    std::cout << it.name << ": " << it.value << std::endl;
//  std::cout << std::endl;
//
//  // Encoders can use table size updates to clear dynamic table.
//  enc.add_table_size_update(0);
//  enc.add_table_size_update(4096);
//
//  serialized_headers = "";
//  enc.encode(send_headers2, serialized_headers);
//  dec.decode(serialized_headers.begin(), serialized_headers.end(), recv_headers2);
//  for (auto it : recv_headers2)
//    std::cout << it.name << ": " << it.value << std::endl;
  //----------------------------------------------------------------//

  //----------------------------------------------------------------//
  // Server Test
  //
  http::router app;
  app.register_handler(std::regex("^/redirect-url$"), [](http::server::request&& req, http::server::response&& res, const std::smatch& matches)
  {
    res.head().status_code(http::status_code::found);
    res.head().header("location","/new-url");
    res.end();
  });
  app.register_handler(std::regex("^/(.*)$"), [](http::server::request&& req, http::server::response&& res, const std::smatch& matches)
  {
    auto res_ptr = std::make_shared<http::server::response>(std::move(res));

    for (auto it : req.head().raw_headers())
      std::cout << it.first << ": " << it.second << std::endl;

    auto req_entity = std::make_shared<std::stringstream>();
    req.on_data([req_entity](const char*const data, std::size_t datasz)
    {
      req_entity->write(data, datasz);
    });

    req.on_end([res_ptr, req_entity]()
    {
      auto push_promise = res_ptr->send_push_promise(http::request_head("/push-url"));

      res_ptr->send("Received: " + req_entity->str());
      res_ptr->end();

      push_promise.fulfill([](http::server::request&& rq, http::server::response&& rs)
      {
        // TODO: have on_end immidiately callback if closed or half closed remote.
        rs.end("Here's the promised data.");
      });

    });

  });


  auto ops = http::server::ssl_options(asio::ssl::context::method::sslv23);
//  {
//    std::ifstream ifs("/Users/jonathonl/Developer/certs/server.key");
//    if (ifs.good())
//      ops.key.assign((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
//  }
//  {
//    std::ifstream ifs("/Users/jonathonl/Developer/certs/server.crt");
//    if (ifs.good())
//      ops.cert.assign((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
//  }
//  {
//    std::ifstream ifs("/Users/jonathonl/Developer/certs/ca.crt");
//    if (ifs.good())
//      ops.ca.assign((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
//  }

  {
    std::ifstream ifs("tests/certs/server.crt");
    if (ifs.good())
      ops.chain.assign((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
  }
  {
    std::ifstream ifs("tests/certs/server.key");
    if (ifs.good())
      ops.key.assign((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
  }
  {
    std::ifstream ifs("tests/certs/dh2048.pem");
    if (ifs.good())
      ops.dhparam.assign((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
  }

  http::server srv(ioservice, ops, 8080, "0.0.0.0");
  srv.listen(std::bind(&http::router::route, &app, std::placeholders::_1, std::placeholders::_2));

  //http::server ssl_srv(ioservice, http::server::ssl_options(asio::ssl::context::method::sslv23), 8081, "0.0.0.0");
  //ssl_srv.listen(std::bind(&http::router::route, &app, std::placeholders::_1, std::placeholders::_2));
  //----------------------------------------------------------------//

  if (true)
  {
    //----------------------------------------------------------------//
    // Client to Local Server Test
    //
    http::client c1(ioservice, "127.0.0.1", http::client::ssl_options(), 8080);
    c1.on_connect([&c1]()
    {
      http::client::request req = c1.make_request();
      req.head() = http::request_head("/foobar", http::method::post,
        {
          {"content-type","application/x-www-form-urlencoded"}
        });

      req.on_response([&c1](http::client::response && resp)
      {
        for (auto it : resp.head().raw_headers())
          std::cout << it.first << ": " << it.second << std::endl;

        if (!(resp.head().status_code() == 200))
        {
          resp.close();
        }
        else
        {
          auto response_data = std::make_shared<std::stringstream>();
          resp.on_data([response_data](const char *const data, std::size_t datasz)
          {
            response_data->write(data, datasz);
          });

          resp.on_end([response_data]()
          {
            std::cout << response_data->rdbuf() << std::endl;
          });
        }
      });

      req.on_push_promise(std::bind(handle_push_promise, std::placeholders::_1, req.stream_id()));

      req.on_close([&c1](http::errc) { /*c1.close();*/ });


      req.end(std::string("name=value&name2=value2"));
    });

    c1.on_close([&ioservice](http::errc ec)
    {
      std::cout << ec << std::endl;
      //ioservice.stop();
    });
    //----------------------------------------------------------------//

    ioservice.run();
  }


  //----------------------------------------------------------------//
  // Client to Google Test
  //
  else if (false)
  {
    http::client c2(ioservice, "www.google.com", http::client::ssl_options());
    c2.on_connect([&c2]()
    {
      if (false)
      {
        auto request = std::make_shared<http::client::request>(c2.make_request());

        request->on_response([request, &c2](http::client::response && resp)
        {
          auto response = std::make_shared<http::client::response>(std::move(resp));


          response->on_data([](const char *const data, std::size_t datasz)
          {
            std::cout << std::string(data, datasz);
          });

          response->on_end([]()
          {
            std::cout << std::endl << "DONE" << std::endl;
          });
        });

        request->on_close([&c2](http::errc error_code)
        {
          c2.close();
        });

        request->head().path("/foobar");
        request->head().method(http::method::post);
        request->head().header("content-type","application/x-www-form-urlencoded");
        request->end("name=value&name2=value2");


      }

      if (false)
      {
        auto req2(c2.make_request());


        auto req_ptr = std::make_shared<http::client::request>(std::move(req2));

        // Create file stream for response.
        auto ofs = std::make_shared<std::ofstream>("./reponse_file.txt.tmp");

        // Set on response handler.
        req_ptr->on_response([&ofs, req_ptr](http::client::response &&res)
        {
          auto res_ptr = std::make_shared<http::client::response>(std::move(res));

          if (res.head().status_code() != 200)
          {
            req_ptr->close();
          }
          else
          {
            // Write response data to file.
            res_ptr->on_data([ofs](const char *const data, std::size_t datasz)
            {
              ofs->write(data, datasz);
            });

            // Close and rename file when done.
            res_ptr->on_end([ofs]()
            {
              ofs->close();
              std::rename("./response_file.txt.tmp","./resonse_file.txt");
            });
          }
        });

        //
        req_ptr->on_close([ofs](http::errc ec)
        {
          ofs->close();
          std::remove("./response_file.txt.tmp");
        });

        req_ptr->on_push_promise(std::bind(handle_push_promise, std::placeholders::_1, req_ptr->stream_id()));

        req_ptr->end();
      }
    });

    c2.on_close([](http::errc ec) { std::cerr << ec << std::endl; });
  }
  //----------------------------------------------------------------//

};
//################################################################//