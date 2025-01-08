#ifndef BAKLAGA_HTTP_MESSAGE__HPP
#define BAKLAGA_HTTP_MESSAGE__HPP

#include <cstdint>
#include <format>
#include <string_view>
#include <system_error>
#include <type_traits>

#include "baklaga/http/detail/string.hpp"
#include "baklaga/http/detail/message.hpp"
#include "baklaga/http/detail/status_code.hpp"

namespace baklaga::http {
using detail::headers_t;
using detail::method_t;
using detail::status_code_t;

enum class message_t { request, response };

template <message_t Type, bool Mutable = false>
class basic_message {
 public:
  using underlying_t =
      std::conditional_t<Mutable, std::string, std::string_view>;

  basic_message() : method_{}, target_{}, version_{}, headers_{} {}
  basic_message(std::string_view buffer) : basic_message{} { parse(buffer); }

  basic_message& operator=(std::string_view buffer) {
    parse(buffer);
    return *this;
  }

  void parse(std::string_view buffer) {
    auto start_line_end = buffer.find("\r\n");
    if (start_line_end == std::string_view::npos) {
      error_ = std::make_error_code(std::errc::bad_message);
      return;
    }

    auto start_line =
        detail::split_view<3>(buffer.substr(0, start_line_end), " ");
    if constexpr (Type == message_t::request) {
      method_ = detail::to_method(start_line[0]);
      if (method_ == detail::type_npos<method_t>()) {
        error_ = std::make_error_code(std::errc::operation_not_supported);
        return;
      }
      target_ = start_line[1];
      version_ = detail::to_version(start_line[2]);
    } else if constexpr (Type == message_t::response) {
      version_ = detail::to_version(start_line[0]);
      std::from_chars(start_line[1].data(),
                      start_line[1].data() + start_line[1].size(),
                      reinterpret_cast<uint16_t&>(status_code_));
      // status_ = start_line[2];
    }

    if (version_ == detail::type_npos<decltype(version_)>()) {
      error_ = std::make_error_code(std::errc::protocol_not_supported);
      return;
    }

    headers_ = detail::to_headers(
        buffer.substr(start_line_end + 2,
                      buffer.find("\r\n\r\n", start_line_end)));
  }

  std::string build() const {
    std::string result{};

    auto version_str = detail::from_version(version_);
    if constexpr (Type == message_t::request) {
      auto method_str = detail::from_method(method_);

      result =
          std::format("{:s} {:s} {:s}\r\n", method_str, target_, version_str);
    } else if constexpr (Type == message_t::response) {
      auto status_str = detail::from_status_code(status_code_);

      result = std::format("{:s} {:d} {:s}\r\n", version_str,
                           static_cast<uint16_t>(status_code_), status_str);
    }

    for (const auto& [name, content] : headers_) {
      result += std::format("{:s}: {:s}\r\n", name, content);
    }

    return result;
  }

  auto method() const noexcept
    requires(Type == message_t::request)
  {
    return method_;
  }
  auto status_code() const
    requires(Type == message_t::response)
  {
    return status_code_;
  }
  auto target() const noexcept
    requires(Type == message_t::request)
  {
    return target_;
  }
  auto version() const noexcept { return version_; }
  const auto& headers() const noexcept { return headers_; }
  const auto& error() const noexcept { return error_; }

  void method(method_t v) noexcept
    requires(Mutable && Type == message_t::request)
  {
    method_ = v;
  }
  auto status_code(status_code_t v) noexcept
    requires(Mutable && Type == message_t::response)
  {
    status_code_ = v;
  }
  void target(std::string_view v) noexcept
    requires(Mutable && Type == message_t::request)
  {
    target_ = v;
  }
  void version(uint8_t v) noexcept
    requires(Mutable)
  {
    version_ = v;
  }
  void headers(const headers_t& v) noexcept
    requires(Mutable)
  {
    headers_ = v;
  }
  auto& headers() noexcept
    requires(Mutable)
  {
    return headers_;
  }
  operator std::string() const { return build(); }

 private:
  method_t method_;
  status_code_t status_code_;
  underlying_t target_;
  uint8_t version_;
  headers_t headers_;
  std::error_code error_;
};

using request_view = basic_message<message_t::request>;
using request = basic_message<message_t::request, true>;
using response_view = basic_message<message_t::response>;
using response = basic_message<message_t::response, true>;
}  // namespace baklaga::http

#endif  // BAKLAGA_HTTP_MESSAGE__HPP
