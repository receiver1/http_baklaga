# HTTP Baklaga
This is a whole baklaga for parsing and sending HTTP requests using C++20.

## Interface
* baklaga::http
  * uri_view
  * uri
  * request_view
  * response_view
  * request
  * response
  * stream\<socket\>
  * get()
  * post()
  * put()
  * delete_()

## Status
* ❔ — not planned (can be done by yourself if required)
* ✔️ — already completed
* ❌ — will be completed

|Feature|Status|
|-|-|
|Persistent connections|❌|
|Pararrel requests|❌|
|Connection states|❌|
|Chunked transfer|❌|
|Status codes|✔️|
|User headers|✔️|
|HTTPS|❔|
|Cookies support|❔|
|Requests caching|❔|
|Content compression|❔|
|Basic authentication|❔|
|Digest authentication|❔|

## Socket
> [!TIP]
> This library was developed for asio so you can easily wrap `asio::ip::tcp::socket`.

To use the library you will need to implement your own socket class, which will be passed to `http::stream`
```cpp
namespace tcp {
class socket {
  public:
    void open();
    void connect();
    void read();
    void write();
    void shutdown();
    void close();
};
}
```

## Example
You can see examples of usage in `/examples` project directory.