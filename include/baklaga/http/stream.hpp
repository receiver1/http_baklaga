#ifndef BAKLAGA_HTTP_STREAM_HPP
#define BAKLAGA_HTTP_STREAM_HPP

#include "baklaga/http/concept/buffer.hpp"
#include "baklaga/http/concept/socket.hpp"
#include "baklaga/http/detail/string.hpp"
#include "baklaga/http/message.hpp"
#include "baklaga/http/uri.hpp"

namespace baklaga::http {
template <concept_::socket Socket>
class stream {
 public:
  stream() = default;
  stream(Socket&& socket) : socket_(std::move(socket)) {}
  ~stream() {}

  std::error_code connect(http::uri_view uri) {
    std::error_code ec;
    socket_.open(ec);
    if (ec) {
      return ec;
    }

    socket_.connect(ec);
    return ec;
  }
  void write(http::request& request) {
    fill_basic_data(request);
    socket_.write(request.build());
  }
  template <concept_::ReadBuffer BufferTy>
  http::response_view read(BufferTy& buffer) {
    std::error_code ec;

    while (!buffer.contains("\r\n\r\n")) {
      auto bytes_incoming = socket_.available();
      auto end_of_previous_data = buffer.size();
      buffer.resize(buffer.size() + bytes_incoming);

      auto buffer_span = std::span<uint8_t>(buffer);
      auto read_buffer = buffer_span.subspan(end_of_previous_data);
      auto bytes_readed = socket_.read(read_buffer, ec);
      if (bytes_readed == 0 || ec) {
        break;
      }
    }

    http::response_view response{buffer};
    auto [content_length, ec] =
        detail::to_arithmetic<int>(response.headers().at("Content-Length"));
    if (ec) {
      return response;
    }

    buffer.resize(buffer.size() + content_length);
    socket_.read(buffer.data() + content_length, ec);

    return response;
  }
  void shutdown() {}

 private:
  void fill_basic_data(http::request& request) {
    auto& headers = request.headers();
    headers.try_emplace("Host", uri_.authority().hostname());
    headers.try_emplace("Accept", "*/*");
    headers.try_emplace("User-Agent", "baklaga");
    headers.try_emplace("Connection", "close");
  }

  Socket socket_;
  http::uri_view uri_;
};
}  // namespace baklaga::http

#endif  // BAKLAGA_HTTP_STREAM_HPP
