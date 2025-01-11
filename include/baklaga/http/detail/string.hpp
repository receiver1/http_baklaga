#ifndef BAKLAGA_HTTP_DETAIL_STRING_HPP
#define BAKLAGA_HTTP_DETAIL_STRING_HPP

#include <array>
#include <charconv>
#include <ranges>
#include <string_view>
#include <system_error>
#include <type_traits>

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

template <typename T, typename DecayedT = std::decay_t<T>>
  requires(std::is_arithmetic_v<DecayedT> || std::is_enum_v<DecayedT>)
struct convert_result_t {
  DecayedT value;
  std::error_code ec;
};

template <typename T, typename DecayedT = std::decay_t<T>>
  requires(std::is_arithmetic_v<DecayedT>)
[[nodiscard]] constexpr auto to_arithmetic(std::ranges::sized_range auto buffer,
                                           const int base = 10) noexcept {
  convert_result_t<DecayedT> result{};
  std::from_chars(buffer.data(), buffer.data() + buffer.size(), result.value, base);
  return result;
}

template <typename T, typename DecayedT = std::decay_t<T>,
          typename UnderlyingT = std::underlying_type_t<DecayedT>>
  requires(std::is_arithmetic_v<DecayedT> || std::is_enum_v<DecayedT>)
[[nodiscard]] constexpr auto to_arithmetic(std::ranges::sized_range auto buffer,
                                           const int base = 10) noexcept {
  convert_result_t<DecayedT> result{};
  std::from_chars(buffer.data(), buffer.data() + buffer.size(), reinterpret_cast<UnderlyingT&>(result.value), base);
  return result;
}

}  // namespace baklaga::http::detail

#endif  // BAKLAGA_HTTP_DETAIL_STRING_HPP
