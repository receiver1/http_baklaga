#ifndef BAKLAGA_HTTP_CONCEPT_BUFFER_HPP
#define BAKLAGA_HTTP_CONCEPT_BUFFER_HPP

#include <ranges>

namespace baklaga::http::concept_ {
template <typename Ty>
concept ReadBuffer = std::ranges::contiguous_range<Ty> && requires(Ty buf) {
  { buf.resize(size_t{}) } -> std::same_as<void>;
};
}  // namespace baklaga::http::concept_

#endif  // BAKLAGA_HTTP_CONCEPT_BUFFER_HPP
