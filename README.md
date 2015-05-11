# Manifold
A lightweight http/2 library.

In progress.

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
```
