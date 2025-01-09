#ifndef BAKLAGA_HTTP_STREAM_HPP
#define BAKLAGA_HTTP_STREAM_HPP

#include "baklaga/http/message.hpp"

namespace baklaga::http {
template <class Socket>
class stream {
 public:
  stream(Socket&& socket) : socket_(std::move(socket)) {}
  ~stream() { socket_.close(); }

  void connect() {}
  void request(const http::request& request);
  void shutdown() {}

 private:
  Socket socket_;
};
}  // namespace baklaga::http

#endif  // BAKLAGA_HTTP_STREAM_HPP
