#ifndef BAKLAGA_HTTP_DETAIL_STRING_HPP
#define BAKLAGA_HTTP_DETAIL_STRING_HPP

#include <array>
#include <ranges>
#include <string_view>

namespace baklaga::http::detail {
template <std::size_t N>
  requires(N > 0)
[[nodiscard]] constexpr auto split_view(std::ranges::view auto buffer,
                                        std::string_view delim) noexcept {
  std::array<std::string_view, N> result{};
  size_t count = 0;

  for (auto part : buffer | std::views::split(delim)) {
    if (count >= N)
      break;

    result[count++] =
        std::string_view{std::ranges::begin(part), std::ranges::end(part)};
  }

  return result;
}
}  // namespace baklaga::http::detail

#endif  // BAKLAGA_HTTP_DETAIL_STRING_HPP
