
#include "asio.hpp"
#include "http_server.hpp"
#include "http_client.hpp"
#include "http_router.hpp"



using namespace manifold;

//================================================================//
class my_request_class
{
private:
  http::client& client_;
  std::unique_ptr<http::client::request> request_;
  std::unique_ptr<http::client::response> response_;
public:
  my_request_class(http::client& c) : client_(c) {}
  void handle_response(http::client::response &&res)
  {
    this->response_ = std::unique_ptr<http::client::response>(new http::client::response(std::move(res)));

    this->response_->on_data([](const char *const data, std::size_t datasz)
    {

    });

    this->response_->on_end([]()
    {

    });
  }
  void send()
  {
    this->client_.make_request(http::request_head("/foobar", "post", {{"content-type","application/x-www-form-urlencoded"}}), [this](http::client::request&& req)
    {
      this->request_ = std::unique_ptr<http::client::request>(new http::client::request(std::move(req)));

      this->request_->on_response(std::bind(&my_request_class::handle_response, this, std::placeholders::_1));


      this->request_->end(std::string("name=value&name2=value2"));
    });
  }
};
//================================================================//

//################################################################//
int main()
{
  asio::io_service ioservice;

  //----------------------------------------------------------------//
  // HPack Test
  //
  std::size_t default_table_size = 4096;
  hpack::encoder enc(default_table_size);
  hpack::decoder dec(default_table_size);

  std::list<hpack::header_field> sendHeaders;
  std::list<hpack::header_field> recvHeaders;

  sendHeaders.push_back(hpack::header_field(":path","/"));
  sendHeaders.push_back(hpack::header_field(":method","GET"));
  sendHeaders.push_back(hpack::header_field("content-type","application/json; charset=utf8"));
  sendHeaders.push_back(hpack::header_field("content-length","30"));
  sendHeaders.push_back(hpack::header_field("custom-header","foobar; baz"));
  sendHeaders.push_back(hpack::header_field("custom-header2","NOT INDEXED", hpack::cacheability::no));

  // Clear dynamic table with table size update.
  enc.add_table_size_update(0);
  enc.add_table_size_update(4096);

  std::string serializedHeaders;
  enc.encode(sendHeaders, serializedHeaders);
  dec.decode(serializedHeaders.begin(), serializedHeaders.end(), recvHeaders);

  for (auto it : recvHeaders)
    std::cout << it.name << ": " << it.value << std::endl;
  //----------------------------------------------------------------//

  //----------------------------------------------------------------//
  // Server Test
  //
  http::router app;
  app.register_handler(std::regex("^/(.*)$"), [](http::server::request&& req, http::server::response&& res, const std::smatch& matches)
  {
    auto req_ptr = std::make_shared<http::server::request>(std::move(req));
    auto res_ptr = std::make_shared<http::server::response>(std::move(res));

    req_ptr->on_data([](const char*const data, std::size_t datasz)
    {

    });

    req_ptr->on_end([res_ptr]()
    {
      res_ptr->end(std::string("Hello World"));
    });

  });

  http::server srv(ioservice, 8080, "0.0.0.0");
  srv.listen(std::bind(&http::router::route, &app, std::placeholders::_1, std::placeholders::_2));
  //----------------------------------------------------------------//

  //----------------------------------------------------------------//
  // Client to Local Server Test
  //
  http::client c1(ioservice, "localhost", 8080);
  my_request_class r(c1);
  r.send();
  //----------------------------------------------------------------//

  //----------------------------------------------------------------//
  // Client to Google Test
  //
  http::client c2(ioservice, "www.google.com", http::client::ssl_options());

  c2.make_request(http::request_head(), [](http::client::request&& req)
  {
    auto req_ptr = std::make_shared<http::client::request>(std::move(req));

    req_ptr->on_response([](http::client::response &&res)
    {
      auto res_ptr = std::make_shared<http::client::response>(std::move(res));

      res_ptr->on_data([](const char *const data, std::size_t datasz)
      {

      });

      res_ptr->on_end([]()
      {

      });

    });


    req_ptr->end();
  });
  //----------------------------------------------------------------//



  ioservice.run();

  return 0;
}
//################################################################//