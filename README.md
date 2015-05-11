# Manifold
A lightweight http/2 library.

In progress.

## HPACK Usage
HPACK compression can be used independently.

```C++
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
    
    // Encoders can use table size updates to clear dynamic table.
    enc.add_table_size_update(0);
    enc.add_table_size_update(4096);
    
    std::string serializedHeaders;
    enc.encode(sendHeaders, serializedHeaders);
    dec.decode(serializedHeaders.begin(), serializedHeaders.end(), recvHeaders);
```
