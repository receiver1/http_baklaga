#ifndef BAKLAGA_HTTP_DETAIL_MESSAGE_HPP
#define BAKLAGA_HTTP_DETAIL_MESSAGE_HPP

#include <array>
#include <cstdint>
#include <limits>
#include <ranges>
#include <string_view>
#include <unordered_map>

namespace baklaga::http::detail {

[[maybe_unused]] constexpr std::string_view crlf_delimiter = "\r\n";

enum class method_t : uint8_t { get, post, put, delete_ };

using headers_t = std::unordered_map<std::string_view, std::string_view>;
template <typename Ty>
concept HasNumericLimits = std::numeric_limits<Ty>::is_specialized;

template <HasNumericLimits Ty>
[[nodiscard]] constexpr Ty type_npos() noexcept {
  return std::numeric_limits<Ty>::max();
}

template <typename Ty, typename UnderlyingTy = std::underlying_type_t<Ty>>
  requires(std::is_enum_v<Ty> && HasNumericLimits<UnderlyingTy>)
[[nodiscard]] constexpr Ty type_npos() noexcept {
  return static_cast<Ty>(std::numeric_limits<UnderlyingTy>::max());
}

inline constexpr method_t to_method(std::string_view method_str) {
  if (method_str == "GET") {
    return method_t::get;
  } else if (method_str == "POST") {
    return method_t::post;
  } else if (method_str == "PUT") {
    return method_t::put;
  } else if (method_str == "DELETE") {
    return method_t::delete_;
  }
  return detail::type_npos<method_t>();
}

inline constexpr std::string_view from_method(method_t method) {
  switch (method) {
    case method_t::get:
      return "GET";
    case method_t::post:
      return "POST";
    case method_t::put:
      return "PUT";
    case method_t::delete_:
      return "DELETE";
  }
  return {};
}

inline constexpr uint8_t to_version(std::string_view version_str) noexcept {
  if (version_str == "HTTP/1.0") {
    return 10;
  } else if (version_str == "HTTP/1.1") {
    return 11;
  }
  return type_npos<uint8_t>();
}

inline constexpr std::string_view from_version(uint8_t version) noexcept {
  switch (version) {
    case 10:
      return "HTTP/1.0";
    case 11:
      return "HTTP/1.1";
  }
  return {};
}

inline headers_t to_headers(std::string_view headers_str) {
  headers_t headers{};
  for (size_t begin{}, end{}; end != std::string_view::npos;) {
    end = headers_str.find(crlf_delimiter, begin);

    auto header = headers_str.substr(begin, end - begin);
    if (!header.empty()) {
      auto [name, content] = split_view<2>(header, ": ");
      headers[name] = content;
    }

    begin = end + crlf_delimiter.size();
  }
  return headers;
}

}  // namespace baklaga::http::detail

#endif  // BAKLAGA_HTTP_DETAIL_MESSAGE_HPP
