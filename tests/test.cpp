
#include <memory>
#include <fstream>
#include <cstdio>
#include <iomanip>
#include <tuple>

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
void handle_push_promise(http::client::request&& req, std::uint32_t dependency_stream_id)
{

}
//================================================================//

//================================================================//
class client_stream_wrapper
{
private:
  http::client::request req_;
  http::client::response resp_;

  std::function<void()> on_response_headers_fn_;

  void set_response(http::client::response&& r)
  {
    this->resp_ = std::move(r);
  }
public:
  client_stream_wrapper()
    : req_(http::request_head(), nullptr, 0), resp_(http::response_head(), nullptr, 0)
  {}
  client_stream_wrapper(http::client::request&& req)
    : req_(std::move(req)), resp_(http::response_head(), nullptr, 0)
  {
    req_.on_response(std::bind(&client_stream_wrapper::set_response, this, std::placeholders::_1));
  }

  void on_data();
  void on_end();
  void on_close();
  void on_drain();
  void send();
  void end();
  void reset_stream();


};
//================================================================//

//================================================================//
class my_request_class
{
private:
  http::client& c_;
  http::client::request request_;
  http::client::response response_;
public:
  my_request_class(http::client& c) : c_(c), request_(http::request_head(), nullptr, 0), response_(http::response_head(), nullptr, 0)
  {
    this->c_.on_connect(std::bind(&my_request_class::run, this));
  }

  void run()
  {
    this->handle_request(this->c_.make_request());
  }

  void handle_request(http::client::request&& req)
  {
    this->request_ = std::move(req);
    this->request_.head() = http::request_head("/foobar", http::method::post,
      {
        {"content-type","application/x-www-form-urlencoded"}
      });

    this->request_.on_response(std::bind(&my_request_class::handle_response, this, std::placeholders::_1));


    this->request_.end(std::string("name=value&name2=value2"));
  }

  void handle_response(http::client::response &&res)
  {
    this->response_ = std::move(res);

    for (auto it : this->response_.head().raw_headers())
      std::cout << it.name << ": " << it.value << std::endl;

    if (!this->response_.head().is_successful_status())
    {
      this->request_.reset_stream();
    }
    else
    {
      auto response_data = std::make_shared<std::string>();
      this->response_.on_data([response_data](const char *const data, std::size_t datasz)
      {
        response_data->append(data, datasz);
      });

      this->response_.on_end([response_data]()
      {
        std::cout << (*response_data) << std::endl;
      });
    }
  }
};
//================================================================//

//################################################################//
int main()
{
  move_only first;
  move_only second;
  int third;
  std::tie(first, second, third) = return_move_only();

  asio::io_service ioservice;

  //----------------------------------------------------------------//
  std::uint32_t plus_sign_code = (0x7fb << (32 - 11));
  auto res = hpack::huffman_code_tree.find(hpack::huffman_code(plus_sign_code, 32));
  if (res != hpack::huffman_code_tree.end())
    std::cout << res->second << std::endl;

  std::string compressed_literal = {(char)0xf1,(char)0xe3,(char)0xc2,(char)0xe5,(char)0xf2,(char)0x3a,(char)0x6b,(char)0xa0,(char)0xab,(char)0x90,(char)0xf4,(char)0xff};
  for (auto it = compressed_literal.begin(); it != compressed_literal.end(); ++it)
    std::cout << std::hex << (unsigned int)(std::uint8_t)(*it) << std::dec << std::endl;
  std::string uncompressed_literal;
  hpack::decoder::huffman_decode(compressed_literal.begin(), compressed_literal.end(), uncompressed_literal);
  std::cout << "uncompressed size: " << uncompressed_literal.size() << std::endl;
  std::cout << "uncompressed value: " << uncompressed_literal << std::endl;

//  for (auto it = hpack::huffman_code_tree.begin(); it != hpack::huffman_code_tree.end(); ++it)
//    std::cout << "- " << std::hex << it->first.msb_code << std::dec << " | " << it->second << std::endl;

  std::cout.flush();

  //----------------------------------------------------------------//

  //----------------------------------------------------------------//
  // HPack Test
  //
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
    auto req_ptr = std::make_shared<http::server::request>(std::move(req));
    auto res_ptr = std::make_shared<http::server::response>(std::move(res));

    for (auto it : req_ptr->head().raw_headers())
      std::cout << it.name << ": " << it.value << std::endl;

    auto req_entity = std::make_shared<std::string>();
    req_ptr->on_data([req_entity](const char*const data, std::size_t datasz)
    {
      req_entity->append(data, datasz);
    });

    req_ptr->on_end([res_ptr, req_entity]()
    {
      res_ptr->end("Received: " + *req_entity);
    });

  });

  http::server srv(ioservice, 8080, "0.0.0.0");
  srv.listen(std::bind(&http::router::route, &app, std::placeholders::_1, std::placeholders::_2));

  //http::server ssl_srv(ioservice, http::server::ssl_options(asio::ssl::context::method::sslv23), 8081, "0.0.0.0");
  //ssl_srv.listen(std::bind(&http::router::route, &app, std::placeholders::_1, std::placeholders::_2));
  //----------------------------------------------------------------//

  //----------------------------------------------------------------//
  // Client to Local Server Test
  //
  http::client c1(ioservice, "127.0.0.1", 8080);
  c1.on_close([](const std::error_code& ec)
  {
    std::cout << ec.message() << std::endl;
  });
  my_request_class r(c1);
  //----------------------------------------------------------------//

  ioservice.run();

  return 0;

  //----------------------------------------------------------------//
  // Client to Google Test
  //
  http::client c2(ioservice, "www.google.com", http::client::ssl_options());
  c2.on_connect([&c2]()
  {
    auto request = std::make_shared<http::client::request>(c2.make_request());

    request->on_response([request](http::client::response&& resp)
    {
      auto response = std::make_shared<http::client::response>(std::move(resp));

      if (!response->head().is_successful_status())
        request->reset_stream();
      else
      {
        response->on_data([](const char *const data, std::size_t datasz)
        {
          // ...
        });

        response->on_end([]()
        {
          // ...
        });
      }
    });

    request->head().path("/foobar");
    request->head().method(http::method::post);
    request->head().header("content-type","application/x-www-form-urlencoded");
    request->end("name=value&name2=value2");


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
        req_ptr->reset_stream();
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
    req_ptr->on_close([ofs](std::uint32_t ec)
    {
      ofs->close();
      std::remove("./response_file.txt.tmp");
    });

    req_ptr->on_push_promise(std::bind(handle_push_promise, std::placeholders::_1, req_ptr->stream_id()));

    req_ptr->end();
  });

  c2.on_close([](const std::error_code& ec) { std::cerr << ec.message() << std::endl; });
  //----------------------------------------------------------------//



  ioservice.run();

  return 0;
};
//################################################################//