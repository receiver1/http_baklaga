#ifndef BAKLAGA_HTTP_DETAIL_MESSAGE_HPP
#define BAKLAGA_HTTP_DETAIL_MESSAGE_HPP

#include <array>
#include <cstdint>
#include <limits>
#include <string_view>
#include <unordered_map>

namespace baklaga::http::detail {

template <typename Ty>
concept HasNumericLimits = std::numeric_limits<Ty>::is_specialized;

template <HasNumericLimits Ty>
[[nodiscard]] constexpr Ty get_npos() noexcept {
  return std::numeric_limits<Ty>::max();
}

template <typename Ty, typename UnderlyingTy = std::underlying_type_t<Ty>>
requires(std::is_enum_v<Ty>&& HasNumericLimits<UnderlyingTy>)
    [[nodiscard]] constexpr Ty get_npos() noexcept {
  return static_cast<Ty>(std::numeric_limits<UnderlyingTy>::max());
}

using headers_t = std::unordered_map<std::string_view, std::string_view>;

template <std::size_t N>
std::array<std::string_view, N> split_view(std::string_view view,
                                           std::string_view delim) {
  std::array<std::string_view, N> ret{};
  size_t start{0}, end{0}, count{0};

  while (count < N &&
         (end = view.find(delim, start)) != std::string_view::npos) {
    ret[count++] = view.substr(start, end - start);
    start = end + delim.size();
  }

  if (count < N) {
    ret[count] = view.substr(start);
  }

  return ret;
}

uint8_t to_version(std::string_view version_str) {
  if (version_str == "HTTP/1.0") {
    return 10;
  } else if (version_str == "HTTP/1.1") {
    return 11;
  }
  return std::numeric_limits<uint8_t>::max();
}

headers_t to_headers(std::string_view headers_str) {
  headers_t headers{};
  for (size_t begin{}, end{}; end != std::string_view::npos;) {
    end = headers_str.find("\r\n", begin);

    auto header = headers_str.substr(begin, end - begin);
    if (!header.empty()) {
      auto split = detail::split_view<2>(header, ": ");
      headers[split[0]] = split[1];
    }

    begin = end + 2;
  }
  return headers;
}
}  // namespace baklaga::http::detail

#endif  // BAKLAGA_HTTP_DETAIL_MESSAGE_HPP
