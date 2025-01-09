#ifndef BAKLAGA_HTTP_SOCKET_CONCEPT_HPP
#define BAKLAGA_HTTP_SOCKET_CONCEPT_HPP

#include <concepts>
#include <span>
#include <string_view>
#include <system_error>

namespace baklaga::http::concept_ {
template <class Socket>
concept socket = requires(Socket s, std::error_code& error) {
  { s.open(error) } -> std::same_as<void>;
  {
    s.connect(std::string_view{}, std::string_view{}, error)
  } -> std::same_as<void>;
  { s.read(std::span<uint8_t>{}, error) } -> std::same_as<size_t>;
  { s.write(std::span<uint8_t>{}, error) } -> std::same_as<size_t>;
  { s.shutdown(error) } -> std::same_as<void>;
  { s.close(error) } -> std::same_as<void>;
};
}  // namespace baklaga::http::concept_

#endif  // BAKLAGA_HTTP_SOCKET_CONCEPT_HPP
