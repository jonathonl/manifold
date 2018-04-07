# Manifold
An elegant http/2 library using coroutines TS.

In progress.

## Server

```C++
using namespace manifold;
asio::io_service ioservice;
asio::ssl::context ssl_ctx(asio::ssl::context::tlsv12);
// Add certs to context...

http::router app;
app.register_handler(std::regex("^/(.*)$"), [&app](http::server::request req, http::server::response res, std::smatch matches) -> future<void>
{
  std::array<char, 1024> buf;
  std::stringstream req_entity;
  while (req)
  {
    std::size_t datasz = co_await req.recv(buf.data(), buf.size());
    req_entity.send(buf.data(), datasz);
  }

  
  auto push_promise = res.send_push_promise(http::request_head("/main.css"));

  co_await res.end("Received: " + req_entity.str());
  
  push_promise.fulfill(std::ref(app));
});

http::server srv(ioservice, 80, "0.0.0.0");
srv.listen(std::ref(app));

http::server ssl_srv(ioservice, ssl_ctx, 443, "0.0.0.0");
ssl_srv.listen(std::ref(app));

ioservice.run();
```

## Client

```C++
asio::io_service ioservice;
asio::ssl::context ssl_ctx(asio::ssl::context::tlsv12);

http::client user_agent(ioservice, ssl_ctx);
[](http::client& user_agent) -> future<void>
{
  auto req = co_await user_agent.make_request("www.example.com", 8080, http::request_head("/foobar", "POST", {{"content-type","application/x-www-form-urlencoded"}}), ec);
  if (ec)
  {
    // Handle connect error...
  }
  else
  {
    co_await req.end("name=value&name2=value2");
    
    auto res = co_await req.response()
    if (!res.head().has_successful_status())
      res.cancel();
    else
    {
      std::ofstream ofs("/tmp/foobar", std::ios::binary);
      std::array<char, 1024> buf;
      while (res)
      {
        std::size_t datasz = co_await res.recv(buf.data(), buf.size());
        ofs.write(buf.data(), datasz):
      }
    }
  }
}(user_agent);

ioservice.run();
```

## HPACK
HPACK compression can be used independently.

```C++
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

std::list<hpack::header_field> recv_headers;

// Encoders can use table size updates to clear dynamic table.
enc.add_table_size_update(0);
enc.add_table_size_update(4096);

std::string serialized_headers;
enc.encode(send_headers, serialized_headers);
std::cout << serialized_headers.size() << std::endl;
dec.decode(serialized_headers.begin(), serialized_headers.end(), recv_headers);

for (auto it : recv_headers)
    std::cout << it.name << ": " << it.value << std::endl;
```

## Dependencies
* Non-boost version of ASIO.
* OpenSSL.
* Compiler supporting C++ coroutines.