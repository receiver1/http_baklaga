#ifndef BAKLAGA_URI_URI_VIEW_HPP
#define BAKLAGA_URI_URI_VIEW_HPP

#include <cstdint>
#include <string_view>
#include <unordered_map>

namespace baklaga::uri {
/// Parses a URI with minimal memory allocations.
/// Example: scheme://hostname:port/path?query=value#fragment
class uri_view {
 public:
  uri_view(std::string_view buffer) {}

  uri_view& operator=(std::string_view&& buffer) {}

  auto scheme() const noexcept { return scheme_; }
  auto hostname() const noexcept { return hostname_; }
  auto port() const noexcept { return port_; }
  auto path() const noexcept { return path_; }
  auto query() const noexcept { return query_; }
  auto fragment() const noexcept { return fragment_; }

 private:
  std::string_view scheme_;
  std::string_view hostname_;
  uint16_t port_;
  std::string_view path_;
  std::unordered_map<std::string_view, std::string_view> query_;
  std::string_view fragment_;
};
}  // namespace baklaga::uri

#endif  // BAKLAGA_URI_URI_VIEW_HPP
