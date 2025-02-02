#ifndef BAKLAGA_HTTP_URI_HPP
#define BAKLAGA_HTTP_URI_HPP

#include <cstdint>
#include <format>
#include <ranges>
#include <string>
#include <string_view>
#include <unordered_map>

#include "baklaga/http/detail/string.hpp"

namespace baklaga::http {
template <bool Mutable = false>
class basic_uri_authority {
 public:
  using underlying_t =
      std::conditional_t<Mutable, std::string, std::string_view>;

  /// Empty constructor
  basic_uri_authority() = default;

  /// Host constructor
  basic_uri_authority(std::string_view hostname, uint16_t port) : hostname_{hostname}, port_{port} {}

  /// Userinfo constructor
  basic_uri_authority(std::string_view username, std::string_view password, std::string_view hostname, uint16_t port = 0)
      : username_{username}, password_{password}, hostname_{hostname}, port_{port} {}

  /// Parsing constructor
  basic_uri_authority(std::string_view buffer) { parse(buffer); }

  void parse(std::string_view buffer) {
    auto split_by = [](std::string_view str, char delimiter) {
      auto pos = str.find(delimiter);
      if (pos != std::string_view::npos) {
        return std::make_pair(str.substr(0, pos), str.substr(pos + 1));
      }
      return std::make_pair(str, std::string_view{});
    };

    if (auto at_pos = buffer.find('@'); at_pos != std::string_view::npos) {
      auto [userinfo, remaining] = split_by(buffer.substr(0, at_pos), ':');
      username_ = userinfo;
      if (!remaining.empty()) {
        password_ = remaining;
      }
      buffer.remove_prefix(at_pos + 1);
    }

    auto [host, port_str] = split_by(buffer, ':');
    hostname_ = host;
    if (!port_str.empty()) {
      auto [result, _] = detail::to_arithmetic<uint16_t>(port_str);
      port_ = result;
    }
  }

  std::string build() const {
    auto userinfo =
        username_.empty() ? "" : std::format("{}:{}@", username_, password_);
    auto port_str = port_ == 0 ? "" : std::format(":{}", port_);
    return std::format("{}{}{}", userinfo, hostname_, port_str);
  }

  auto username() const noexcept { return username_; }
  auto password() const noexcept { return password_; }
  auto hostname() const noexcept { return hostname_; }
  auto port() const noexcept { return port_; }

  void username(std::string_view v) noexcept
    requires(Mutable)
  {
    username_ = v;
  }
  void password(std::string_view v) noexcept
    requires(Mutable)
  {
    password_ = v;
  }
  void hostname(std::string_view v) noexcept
    requires(Mutable)
  {
    hostname_ = v;
  }
  void port(uint16_t v) noexcept
    requires(Mutable)
  {
    port_ = v;
  }

 private:
  underlying_t username_;
  underlying_t password_;
  underlying_t hostname_;
  uint16_t port_ = 0;
};

/// Parses a URI with minimal memory allocations.
/// Example: scheme://hostname:port/path?query=value#fragment
template <bool Mutable = false>
class basic_uri {
 public:
  using underlying_t =
      std::conditional_t<Mutable, std::string, std::string_view>;

  basic_uri() : scheme_{}, authority_{}, path_{}, query_{}, fragment_{} {}
  basic_uri(std::string_view buffer) : basic_uri{} { parse(buffer); }

  basic_uri& operator=(std::string_view buffer) {
    parse(buffer);
    return *this;
  }

  void parse(std::string_view buffer) {
    auto scheme_end = buffer.find("://");
    if (scheme_end != std::string_view::npos) {
      scheme_ = buffer.substr(0, scheme_end);
      buffer.remove_prefix(scheme_end + 3);
    }

    auto path_start = buffer.find('/');
    authority_ = basic_uri_authority<Mutable>{buffer.substr(0, path_start)};
    if (path_start == std::string_view::npos)
      return;
    buffer.remove_prefix(path_start);

    auto fragment_start = buffer.find('#');
    if (fragment_start != std::string_view::npos) {
      fragment_ = buffer.substr(fragment_start + 1);
      buffer.remove_suffix(buffer.size() - fragment_start);
    }

    auto query_start = buffer.find('?');
    path_ = buffer.substr(0, query_start);
    if (query_start == std::string_view::npos)
      return;

    auto query_str = buffer.substr(query_start + 1);
    for (auto part : query_str | std::views::split('&')) {
      auto [key, value] = detail::split_view<2>(part, "=");
      query_.emplace(key, value);
    }
  }

  std::string build() const {
    auto scheme_str = scheme_.empty() ? "" : scheme_ + "://";

    std::string query_str;
    if (!query_.empty()) {
      query_str = "?";
      for (const auto& [key, value] : query_) {
        if (query_str.size() > 1) {
          query_str.append("&");
        }
        query_str += std::format("{}={}", key, value);
      }
    }

    std::string fragment_str = fragment_.empty() ? "" : "#" + fragment_;

    return std::format("{}{}{}{}{}", scheme_str, authority_.build(), path_,
                       query_str, fragment_str);
  }

  auto scheme() const noexcept { return scheme_; }
  const auto& authority() const noexcept { return authority_; }
  auto path() const noexcept { return path_; }
  const auto& query() const noexcept { return query_; }
  auto fragment() const noexcept { return fragment_; }

  void scheme(std::string_view v) noexcept
    requires(Mutable)
  {
    scheme_ = v;
  }
  auto& authority() noexcept
    requires(Mutable)
  {
    return authority_;
  }
  void path(std::string_view v) noexcept
    requires(Mutable)
  {
    path_ = v;
  }
  auto& query() noexcept
    requires(Mutable)
  {
    return query_;
  }
  void fragment(std::string_view v) noexcept
    requires(Mutable)
  {
    fragment_ = v;
  }

 private:
  underlying_t scheme_;
  basic_uri_authority<Mutable> authority_;
  underlying_t path_;
  std::unordered_map<underlying_t, underlying_t> query_;
  underlying_t fragment_;
};

using uri_view = basic_uri<>;
using uri = basic_uri<true>;
}  // namespace baklaga::http

#endif  // BAKLAGA_HTTP_URI_HPP
