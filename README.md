# Manifold
A lightweight http/2 library.

In progress.

## Server Usage

```C++
asio::io_service ioservice;
http::router app;
app.register_handler(std::regex("^/(.*)$"), [](http::server::request&& req, http::server::response&& res, const std::smatch& matches)
{
  auto req_ptr = std::make_shared<http::server::request>(std::move(req));
  auto res_ptr = std::make_shared<http::server::response>(std::move(res));

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

http::server ssl_srv(ioservice, http::server::ssl_options(asio::ssl::context::method::sslv23), 8081, "0.0.0.0");
ssl_srv.listen(std::bind(&http::router::route, &app, std::placeholders::_1, std::placeholders::_2));
ioservice.run();
```

## HPACK Usage
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
dec.decode(serialized_headers.begin(), serialized_headers.end(), recv_headers);

for (auto it : recv_headers)
    std::cout << it.name << ": " << it.value << std::endl;
```
