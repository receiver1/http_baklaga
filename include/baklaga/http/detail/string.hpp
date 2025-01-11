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

template <typename T, typename DecayedT = std::decay_t<T>,
          bool IsEnum = std::is_enum_v<DecayedT>>
  requires(std::is_arithmetic_v<DecayedT> || IsEnum)
[[nodiscard]] constexpr auto to_arithmetic(std::ranges::sized_range auto buffer,
                                           const int base = 10) noexcept {
  using result_t = convert_result_t<DecayedT>;

  // std::conditional_t with std::underlying_type_t won't work here.
  // conditional_t is a meta function analogue to the regular if,
  // this means that all if paths must be ct evaluated
  // (fun fact: all paths are fully ct evaluated, but it still doesn't work)
  if constexpr (IsEnum) {
    using underlying_t = std::underlying_type_t<DecayedT>;
    underlying_t value{};
    auto [_, ec] = std::from_chars(buffer.data(), buffer.data() + buffer.size(),
                                   value, base);
    return result_t{static_cast<DecayedT>(value), std::make_error_code(ec)};
  } else {
    DecayedT value{};
    auto [_, ec] = std::from_chars(buffer.data(), buffer.data() + buffer.size(),
                                   value, base);
    return result_t{value, std::make_error_code(ec)};
  }
}

}  // namespace baklaga::http::detail

#endif  // BAKLAGA_HTTP_DETAIL_STRING_HPP
