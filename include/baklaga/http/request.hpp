#ifndef BAKLAGA_HTTP_REQUEST_HPP
#define BAKLAGA_HTTP_REQUEST_HPP

#include <cstdint>
#include <string_view>
#include <system_error>

namespace baklaga::http {
enum class method_t : uint8_t { get, post, put, delete_ };

namespace detail {
method_t to_method(std::string_view method_str) {
  if (method_str == "GET") {
    return method_t::get;
  } else if (method_str == "POST") {
    return method_t::post;
  } else if (method_str == "PUT") {
    return method_t::put;
  } else if (method_str == "DELETE") {
    return method_t::delete_;
  }
  return static_cast<method_t>(-1);
}
}

class request {
 public:
  request(std::string_view buffer)
      : method_{}, target_{}, version_{}, headers_{} {
    parse_(buffer);
  }

  request& operator=(std::string_view&& buffer) {
    parse_(buffer);
    return *this;
  }

  auto method() const { return method_; }
  auto target() const { return target_; }
  auto version() const { return version_; }
  auto& headers() { return headers_; }
  const auto& error() const { return error_; }

 private:
  void parse_(std::string_view buffer);

  method_t method_;
  std::string_view target_;
  uint8_t version_;
  headers_t headers_;
  std::error_code error_;
};
}

#include "baklaga/http/impl/request.ipp"

#endif  // BAKLAGA_HTTP_REQUEST_HPP
