#ifndef BAKLAGA_HTTP_STREAM_HPP
#define BAKLAGA_HTTP_STREAM_HPP

#include "baklaga/http/concept/socket.hpp"
#include "baklaga/http/message.hpp"
#include "baklaga/http/uri.hpp"

namespace baklaga::http {
template <concept_::socket Socket>
class stream {
 public:
  stream() = default;
  stream(Socket&& socket) : socket_(std::move(socket)) {}
  ~stream() { }

  void connect(http::uri_view uri) {}
  void write(http::request request);
  http::response_view read();
  void shutdown() {}

 private:
  Socket socket_;
};
}  // namespace baklaga::http

#endif  // BAKLAGA_HTTP_STREAM_HPP
