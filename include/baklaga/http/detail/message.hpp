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

template <std::size_t N>
  requires(N > 0)
[[nodiscard]] constexpr auto split_view(std::string_view view,
                                        std::string_view delim) noexcept {
  std::array<std::string_view, N> result{};
  size_t count = 0;

  for (auto part : view | std::views::split(delim)) {
    if (count >= N)
      break;

    result[count++] =
        std::string_view{std::ranges::begin(part), std::ranges::end(part)};
  }

  return result;
}

inline uint8_t to_version(std::string_view version_str) noexcept {
  if (version_str == "HTTP/1.0") {
    return 10;
  } else if (version_str == "HTTP/1.1") {
    return 11;
  }
  return type_npos<uint8_t>();
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
