#ifndef BAKLAGA_HTTP_URI_ENCODE_HPP
#define BAKLAGA_HTTP_URI_ENCODE_HPP

#include <format>
#include <string>
#include <string_view>

namespace baklaga::http {
inline std::string uri_encode(std::string_view buffer) {
  std::string result{};
  result.reserve(buffer.size() * 3);
  for (auto c : buffer) {
    if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
      result.push_back(c);
    } else {
      result.append(std::format("%{:02x}", static_cast<uint8_t>(c)));
    }
  }
  return result;
}

inline std::string uri_decode(std::string_view buffer) {
  std::string result{};
  result.reserve(buffer.size());
  for (size_t i = 0; i < buffer.size(); ++i) {
    if (buffer[i] == '%' && i + 2 < buffer.size()) {
      auto hex = buffer.substr(i + 1, 2);
      uint8_t c{};
      std::from_chars(hex.data(), hex.data() + hex.size(), c, 16);
      result.push_back(c);
      i += 2;
    } else {
      result.push_back(buffer[i]);
    }
  }

  return result;
}
}  // namespace baklaga::http

#endif  // BAKLAGA_HTTP_URI_ENCODE_HPP
