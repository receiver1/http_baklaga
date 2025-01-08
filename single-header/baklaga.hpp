#ifndef BAKLAGA_HTTP_HPP
#define BAKLAGA_HTTP_HPP

#ifndef BAKLAGA_URI_URI_VIEW_HPP
#define BAKLAGA_URI_URI_VIEW_HPP

#include <charconv>
#include <cstdint>
#include <format>
#include <numeric>
#include <ranges>
#include <string>
#include <string_view>
#include <unordered_map>

#ifndef BAKLAGA_HTTP_DETAIL_STRING_HPP
#define BAKLAGA_HTTP_DETAIL_STRING_HPP

#include <ranges>
#include <array>
#include <string_view>

namespace baklaga::http::detail {
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
}  // namespace baklaga::http::string

#endif  // BAKLAGA_HTTP_DETAIL_STRING_HPP

namespace baklaga::http {
template <bool Mutable = false>
class basic_uri_authority {
 public:
  using underlying_t =
      std::conditional_t<Mutable, std::string, std::string_view>;

  basic_uri_authority() : username_(), password_(), hostname_(), port_{} {}
  basic_uri_authority(std::string_view buffer) : basic_uri_authority{} {
    parse(buffer);
  }

  void parse(std::string_view buffer) {
    auto at_pos = buffer.find('@');
    if (at_pos != std::string_view::npos) {
      auto userinfo = buffer.substr(0, at_pos);
      auto colon_pos = userinfo.find(':');
      if (colon_pos != std::string_view::npos) {
        username_ = userinfo.substr(0, colon_pos);
        password_ = userinfo.substr(colon_pos + 1);
      } else {
        username_ = userinfo;
      }
      buffer.remove_prefix(at_pos + 1);
    }

    auto colon_pos = buffer.find(':');
    if (colon_pos != std::string_view::npos) {
      hostname_ = buffer.substr(0, colon_pos);
      auto port_str = buffer.substr(colon_pos + 1);
      std::from_chars(port_str.data(), port_str.data() + port_str.size(),
                      port_);
    } else {
      hostname_ = buffer;
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
    if (query_start != std::string_view::npos) {
      auto query_str = buffer.substr(query_start + 1);
      for (auto part : query_str | std::views::split('&')) {
        auto [key, value] =
            detail::split_view<2>(std::string_view(std::ranges::begin(part),
                                                   std::ranges::end(part)),
                                  "=");
        query_.emplace(key, value);
      }
    }
  }

  std::string build() const {
    auto scheme_str = scheme_.empty() ? "" : scheme_ + "://";
    auto query_str =
        query_.size() == 0
            ? ""
            : "?" + std::accumulate(std::next(query_.begin()), query_.end(),
                                    std::string{query_.begin()->first} + "=" +
                                        query_.begin()->second,
                                    [](const auto& a, const auto& b) {
                                      return a + "&" + b.first + "=" + b.second;
                                    });
    auto fragment_str = fragment_.empty() ? "" : "#" + fragment_;

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

#endif  // BAKLAGA_URI_URI_VIEW_HPP

#ifndef BAKLAGA_HTTP_MESSAGE__HPP
#define BAKLAGA_HTTP_MESSAGE__HPP

#include <cstdint>
#include <format>
#include <string_view>
#include <system_error>
#include <type_traits>

#ifndef BAKLAGA_HTTP_DETAIL_STRING_HPP
#define BAKLAGA_HTTP_DETAIL_STRING_HPP

#include <ranges>
#include <array>
#include <string_view>

namespace baklaga::http::detail {
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
}  // namespace baklaga::http::string

#endif  // BAKLAGA_HTTP_DETAIL_STRING_HPP

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

enum class method_t : uint8_t { get, post, put, delete_ };

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

inline constexpr method_t to_method(std::string_view method_str) {
  if (method_str == "GET") {
    return method_t::get;
  } else if (method_str == "POST") {
    return method_t::post;
  } else if (method_str == "PUT") {
    return method_t::put;
  } else if (method_str == "DELETE") {
    return method_t::delete_;
  }
  return detail::type_npos<method_t>();
}

inline constexpr std::string_view from_method(method_t method) {
  switch (method) {
    case method_t::get:
      return "GET";
    case method_t::post:
      return "POST";
    case method_t::put:
      return "PUT";
    case method_t::delete_:
      return "DELETE";
  }
  return {};
}

inline constexpr uint8_t to_version(std::string_view version_str) noexcept {
  if (version_str == "HTTP/1.0") {
    return 10;
  } else if (version_str == "HTTP/1.1") {
    return 11;
  }
  return type_npos<uint8_t>();
}

inline constexpr std::string_view from_version(uint8_t version) noexcept {
  switch (version) {
    case 10:
      return "HTTP/1.0";
    case 11:
      return "HTTP/1.1";
  }
  return {};
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

#ifndef BAKLAGA_HTTP_DETAIL_STATUS_CODE_HPP
#define BAKLAGA_HTTP_DETAIL_STATUS_CODE_HPP

#include <array>
#include <cstdint>
#include <string_view>

namespace baklaga::http::detail {
enum class status_code_t : uint16_t {
  // Informational
  continue_ = 100,
  switching_protocols = 101,
  processing = 102,
  early_hints = 103,

  // Successful
  ok = 200,
  created = 201,
  accepted = 202,
  non_authoritative_information = 203,
  no_content = 204,
  reset_content = 205,
  partial_content = 206,
  multi_status = 207,
  already_reported = 208,
  im_used = 226,

  // Redirection
  multiple_choices = 300,
  moved_permanently = 301,
  found = 302,
  see_other = 303,
  not_modified = 304,
  use_proxy = 305,
  temporary_redirect = 307,
  permanent_redirect = 308,

  // Client Error
  bad_request = 400,
  unauthorized = 401,
  payment_required = 402,
  forbidden = 403,
  not_found = 404,
  method_not_allowed = 405,
  not_acceptable = 406,
  proxy_authentication_required = 407,
  request_timeout = 408,
  conflict = 409,
  gone = 410,
  length_required = 411,
  precondition_failed = 412,
  payload_too_large = 413,
  uri_too_long = 414,
  unsupported_media_type = 415,
  range_not_satisfiable = 416,
  expectation_failed = 417,
  im_a_teapot = 418,
  misdirected_request = 421,
  unprocessable_entity = 422,
  locked = 423,
  failed_dependency = 424,
  upgrade_required = 426,
  precondition_required = 428,
  too_many_requests = 429,
  request_header_fields_too_large = 431,
  unavailable_for_legal_reasons = 451,

  // Server Error
  internal_server_error = 500,
  not_implemented = 501,
  bad_gateway = 502,
  service_unavailable = 503,
  gateway_timeout = 504,
  http_version_not_supported = 505,
  variant_also_negotiates = 506,
  insufficient_storage = 507,
  loop_detected = 508,
  not_extended = 510,
  network_authentication_required = 511
};

constexpr std::array<std::pair<status_code_t, std::string_view>, 61>
    status_code_map = {
        {{status_code_t::continue_, "Continue"},
         {status_code_t::switching_protocols, "Switching Protocols"},
         {status_code_t::processing, "Processing"},
         {status_code_t::early_hints, "Early Hints"},
         {status_code_t::ok, "OK"},
         {status_code_t::created, "Created"},
         {status_code_t::accepted, "Accepted"},
         {status_code_t::non_authoritative_information,
          "Non Authoritative Information"},
         {status_code_t::no_content, "No Content"},
         {status_code_t::reset_content, "Reset Content"},
         {status_code_t::partial_content, "Partial Content"},
         {status_code_t::multi_status, "Multi-Status"},
         {status_code_t::already_reported, "Already Reported"},
         {status_code_t::im_used, "IM Used"},
         {status_code_t::multiple_choices, "Multiple Choices"},
         {status_code_t::moved_permanently, "Moved Permanently"},
         {status_code_t::found, "Found"},
         {status_code_t::see_other, "See Other"},
         {status_code_t::not_modified, "Not Modified"},
         {status_code_t::use_proxy, "Use Proxy"},
         {status_code_t::temporary_redirect, "Temporary Redirect"},
         {status_code_t::permanent_redirect, "Permanent Redirect"},
         {status_code_t::bad_request, "Bad Request"},
         {status_code_t::unauthorized, "Unauthorized"},
         {status_code_t::payment_required, "Payment Required"},
         {status_code_t::forbidden, "Forbidden"},
         {status_code_t::not_found, "Not Found"},
         {status_code_t::method_not_allowed, "Method Not Allowed"},
         {status_code_t::not_acceptable, "Not Acceptable"},
         {status_code_t::proxy_authentication_required,
          "Proxy Authentication Required"},
         {status_code_t::request_timeout, "Request Timeout"},
         {status_code_t::conflict, "Conflict"},
         {status_code_t::gone, "Gone"},
         {status_code_t::length_required, "Length Required"},
         {status_code_t::precondition_failed, "Precondition Failed"},
         {status_code_t::payload_too_large, "Payload Too Large"},
         {status_code_t::uri_too_long, "URI Too Long"},
         {status_code_t::unsupported_media_type, "Unsupported Media Type"},
         {status_code_t::range_not_satisfiable, "Range Not Satisfiable"},
         {status_code_t::expectation_failed, "Expectation Failed"},
         {status_code_t::im_a_teapot, "I'm a teapot"},
         {status_code_t::misdirected_request, "Misdirected Request"},
         {status_code_t::unprocessable_entity, "Unprocessable Entity"},
         {status_code_t::locked, "Locked"},
         {status_code_t::failed_dependency, "Failed Dependency"},
         {status_code_t::upgrade_required, "Upgrade Required"},
         {status_code_t::precondition_required, "Precondition Required"},
         {status_code_t::too_many_requests, "Too Many Requests"},
         {status_code_t::request_header_fields_too_large,
          "Request Header Fields Too Large"},
         {status_code_t::unavailable_for_legal_reasons,
          "Unavailable For Legal Reasons"},
         {status_code_t::internal_server_error, "Internal Server Error"},
         {status_code_t::not_implemented, "Not Implemented"},
         {status_code_t::bad_gateway, "Bad Gateway"},
         {status_code_t::service_unavailable, "Service Unavailable"},
         {status_code_t::gateway_timeout, "Gateway Timeout"},
         {status_code_t::http_version_not_supported,
          "HTTP Version Not Supported"},
         {status_code_t::variant_also_negotiates, "Variant Also Negotiates"},
         {status_code_t::insufficient_storage, "Insufficient Storage"},
         {status_code_t::loop_detected, "Loop Detected"},
         {status_code_t::not_extended, "Not Extended"},
         {status_code_t::network_authentication_required,
          "Network Authentication Required"}}};

inline constexpr std::string_view from_status_code(status_code_t status_code) {
  auto it = std::find_if(status_code_map.begin(), status_code_map.end(),
                         [status_code](auto& pair) {
                           return pair.first == status_code;
                         });
  if (it != status_code_map.end()) {
    return it->second;
  }
  return {};
}
}  // namespace baklaga::http::detail

#endif  // BAKLAGA_HTTP_DETAIL_STATUS_CODE_HPP

namespace baklaga::http {
using detail::headers_t;
using detail::method_t;
using detail::status_code_t;

enum class message_t { request, response };

template <message_t Type, bool Mutable = false>
class basic_message {
 public:
  using underlying_t =
      std::conditional_t<Mutable, std::string, std::string_view>;

  basic_message() : method_{}, target_{}, version_{}, headers_{} {}
  basic_message(std::string_view buffer) : basic_message{} { parse(buffer); }

  basic_message& operator=(std::string_view buffer) {
    parse(buffer);
    return *this;
  }

  void parse(std::string_view buffer) {
    auto start_line_end = buffer.find("\r\n");
    if (start_line_end == std::string_view::npos) {
      error_ = std::make_error_code(std::errc::bad_message);
      return;
    }

    auto start_line =
        detail::split_view<3>(buffer.substr(0, start_line_end), " ");
    if constexpr (Type == message_t::request) {
      method_ = detail::to_method(start_line[0]);
      if (method_ == detail::type_npos<method_t>()) {
        error_ = std::make_error_code(std::errc::operation_not_supported);
        return;
      }
      target_ = start_line[1];
      version_ = detail::to_version(start_line[2]);
    } else if constexpr (Type == message_t::response) {
      version_ = detail::to_version(start_line[0]);
      std::from_chars(start_line[1].data(),
                      start_line[1].data() + start_line[1].size(),
                      reinterpret_cast<uint16_t&>(status_code_));
      // status_ = start_line[2];
    }

    if (version_ == detail::type_npos<decltype(version_)>()) {
      error_ = std::make_error_code(std::errc::protocol_not_supported);
      return;
    }

    headers_ = detail::to_headers(
        buffer.substr(start_line_end + 2,
                      buffer.find("\r\n\r\n", start_line_end)));
  }

  std::string build() const {
    std::string result{};

    auto version_str = detail::from_version(version_);
    if constexpr (Type == message_t::request) {
      auto method_str = detail::from_method(method_);

      result =
          std::format("{:s} {:s} {:s}\r\n", method_str, target_, version_str);
    } else if constexpr (Type == message_t::response) {
      auto status_str = detail::from_status_code(status_code_);

      result = std::format("{:s} {:d} {:s}\r\n", version_str,
                           static_cast<uint16_t>(status_code_), status_str);
    }

    for (const auto& [name, content] : headers_) {
      result += std::format("{:s}: {:s}\r\n", name, content);
    }

    return result;
  }

  auto method() const noexcept
    requires(Type == message_t::request)
  {
    return method_;
  }
  auto status_code() const
    requires(Type == message_t::response)
  {
    return status_code_;
  }
  auto target() const noexcept
    requires(Type == message_t::request)
  {
    return target_;
  }
  auto version() const noexcept { return version_; }
  const auto& headers() const noexcept { return headers_; }
  const auto& error() const noexcept { return error_; }

  void method(method_t v) noexcept
    requires(Mutable && Type == message_t::request)
  {
    method_ = v;
  }
  auto status_code(status_code_t v) noexcept
    requires(Mutable && Type == message_t::response)
  {
    status_code_ = v;
  }
  void target(std::string_view v) noexcept
    requires(Mutable && Type == message_t::request)
  {
    target_ = v;
  }
  void version(uint8_t v) noexcept
    requires(Mutable)
  {
    version_ = v;
  }
  void headers(const headers_t& v) noexcept
    requires(Mutable)
  {
    headers_ = v;
  }
  auto& headers() noexcept
    requires(Mutable)
  {
    return headers_;
  }
  operator std::string() const { return build(); }

 private:
  method_t method_;
  status_code_t status_code_;
  underlying_t target_;
  uint8_t version_;
  headers_t headers_;
  std::error_code error_;
};

using request_view = basic_message<message_t::request>;
using request = basic_message<message_t::request, true>;
using response_view = basic_message<message_t::response>;
using response = basic_message<message_t::response, true>;
}  // namespace baklaga::http

#endif  // BAKLAGA_HTTP_MESSAGE__HPP

#ifndef BAKLAGA_HTTP_STREAM_HPP
#define BAKLAGA_HTTP_STREAM_HPP

#ifndef BAKLAGA_HTTP_MESSAGE__HPP
#define BAKLAGA_HTTP_MESSAGE__HPP

#include <cstdint>
#include <format>
#include <string_view>
#include <system_error>
#include <type_traits>

#ifndef BAKLAGA_HTTP_DETAIL_STRING_HPP
#define BAKLAGA_HTTP_DETAIL_STRING_HPP

#include <ranges>
#include <array>
#include <string_view>

namespace baklaga::http::detail {
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
}  // namespace baklaga::http::string

#endif  // BAKLAGA_HTTP_DETAIL_STRING_HPP

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

enum class method_t : uint8_t { get, post, put, delete_ };

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

inline constexpr method_t to_method(std::string_view method_str) {
  if (method_str == "GET") {
    return method_t::get;
  } else if (method_str == "POST") {
    return method_t::post;
  } else if (method_str == "PUT") {
    return method_t::put;
  } else if (method_str == "DELETE") {
    return method_t::delete_;
  }
  return detail::type_npos<method_t>();
}

inline constexpr std::string_view from_method(method_t method) {
  switch (method) {
    case method_t::get:
      return "GET";
    case method_t::post:
      return "POST";
    case method_t::put:
      return "PUT";
    case method_t::delete_:
      return "DELETE";
  }
  return {};
}

inline constexpr uint8_t to_version(std::string_view version_str) noexcept {
  if (version_str == "HTTP/1.0") {
    return 10;
  } else if (version_str == "HTTP/1.1") {
    return 11;
  }
  return type_npos<uint8_t>();
}

inline constexpr std::string_view from_version(uint8_t version) noexcept {
  switch (version) {
    case 10:
      return "HTTP/1.0";
    case 11:
      return "HTTP/1.1";
  }
  return {};
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

#ifndef BAKLAGA_HTTP_DETAIL_STATUS_CODE_HPP
#define BAKLAGA_HTTP_DETAIL_STATUS_CODE_HPP

#include <array>
#include <cstdint>
#include <string_view>

namespace baklaga::http::detail {
enum class status_code_t : uint16_t {
  // Informational
  continue_ = 100,
  switching_protocols = 101,
  processing = 102,
  early_hints = 103,

  // Successful
  ok = 200,
  created = 201,
  accepted = 202,
  non_authoritative_information = 203,
  no_content = 204,
  reset_content = 205,
  partial_content = 206,
  multi_status = 207,
  already_reported = 208,
  im_used = 226,

  // Redirection
  multiple_choices = 300,
  moved_permanently = 301,
  found = 302,
  see_other = 303,
  not_modified = 304,
  use_proxy = 305,
  temporary_redirect = 307,
  permanent_redirect = 308,

  // Client Error
  bad_request = 400,
  unauthorized = 401,
  payment_required = 402,
  forbidden = 403,
  not_found = 404,
  method_not_allowed = 405,
  not_acceptable = 406,
  proxy_authentication_required = 407,
  request_timeout = 408,
  conflict = 409,
  gone = 410,
  length_required = 411,
  precondition_failed = 412,
  payload_too_large = 413,
  uri_too_long = 414,
  unsupported_media_type = 415,
  range_not_satisfiable = 416,
  expectation_failed = 417,
  im_a_teapot = 418,
  misdirected_request = 421,
  unprocessable_entity = 422,
  locked = 423,
  failed_dependency = 424,
  upgrade_required = 426,
  precondition_required = 428,
  too_many_requests = 429,
  request_header_fields_too_large = 431,
  unavailable_for_legal_reasons = 451,

  // Server Error
  internal_server_error = 500,
  not_implemented = 501,
  bad_gateway = 502,
  service_unavailable = 503,
  gateway_timeout = 504,
  http_version_not_supported = 505,
  variant_also_negotiates = 506,
  insufficient_storage = 507,
  loop_detected = 508,
  not_extended = 510,
  network_authentication_required = 511
};

constexpr std::array<std::pair<status_code_t, std::string_view>, 61>
    status_code_map = {
        {{status_code_t::continue_, "Continue"},
         {status_code_t::switching_protocols, "Switching Protocols"},
         {status_code_t::processing, "Processing"},
         {status_code_t::early_hints, "Early Hints"},
         {status_code_t::ok, "OK"},
         {status_code_t::created, "Created"},
         {status_code_t::accepted, "Accepted"},
         {status_code_t::non_authoritative_information,
          "Non Authoritative Information"},
         {status_code_t::no_content, "No Content"},
         {status_code_t::reset_content, "Reset Content"},
         {status_code_t::partial_content, "Partial Content"},
         {status_code_t::multi_status, "Multi-Status"},
         {status_code_t::already_reported, "Already Reported"},
         {status_code_t::im_used, "IM Used"},
         {status_code_t::multiple_choices, "Multiple Choices"},
         {status_code_t::moved_permanently, "Moved Permanently"},
         {status_code_t::found, "Found"},
         {status_code_t::see_other, "See Other"},
         {status_code_t::not_modified, "Not Modified"},
         {status_code_t::use_proxy, "Use Proxy"},
         {status_code_t::temporary_redirect, "Temporary Redirect"},
         {status_code_t::permanent_redirect, "Permanent Redirect"},
         {status_code_t::bad_request, "Bad Request"},
         {status_code_t::unauthorized, "Unauthorized"},
         {status_code_t::payment_required, "Payment Required"},
         {status_code_t::forbidden, "Forbidden"},
         {status_code_t::not_found, "Not Found"},
         {status_code_t::method_not_allowed, "Method Not Allowed"},
         {status_code_t::not_acceptable, "Not Acceptable"},
         {status_code_t::proxy_authentication_required,
          "Proxy Authentication Required"},
         {status_code_t::request_timeout, "Request Timeout"},
         {status_code_t::conflict, "Conflict"},
         {status_code_t::gone, "Gone"},
         {status_code_t::length_required, "Length Required"},
         {status_code_t::precondition_failed, "Precondition Failed"},
         {status_code_t::payload_too_large, "Payload Too Large"},
         {status_code_t::uri_too_long, "URI Too Long"},
         {status_code_t::unsupported_media_type, "Unsupported Media Type"},
         {status_code_t::range_not_satisfiable, "Range Not Satisfiable"},
         {status_code_t::expectation_failed, "Expectation Failed"},
         {status_code_t::im_a_teapot, "I'm a teapot"},
         {status_code_t::misdirected_request, "Misdirected Request"},
         {status_code_t::unprocessable_entity, "Unprocessable Entity"},
         {status_code_t::locked, "Locked"},
         {status_code_t::failed_dependency, "Failed Dependency"},
         {status_code_t::upgrade_required, "Upgrade Required"},
         {status_code_t::precondition_required, "Precondition Required"},
         {status_code_t::too_many_requests, "Too Many Requests"},
         {status_code_t::request_header_fields_too_large,
          "Request Header Fields Too Large"},
         {status_code_t::unavailable_for_legal_reasons,
          "Unavailable For Legal Reasons"},
         {status_code_t::internal_server_error, "Internal Server Error"},
         {status_code_t::not_implemented, "Not Implemented"},
         {status_code_t::bad_gateway, "Bad Gateway"},
         {status_code_t::service_unavailable, "Service Unavailable"},
         {status_code_t::gateway_timeout, "Gateway Timeout"},
         {status_code_t::http_version_not_supported,
          "HTTP Version Not Supported"},
         {status_code_t::variant_also_negotiates, "Variant Also Negotiates"},
         {status_code_t::insufficient_storage, "Insufficient Storage"},
         {status_code_t::loop_detected, "Loop Detected"},
         {status_code_t::not_extended, "Not Extended"},
         {status_code_t::network_authentication_required,
          "Network Authentication Required"}}};

inline constexpr std::string_view from_status_code(status_code_t status_code) {
  auto it = std::find_if(status_code_map.begin(), status_code_map.end(),
                         [status_code](auto& pair) {
                           return pair.first == status_code;
                         });
  if (it != status_code_map.end()) {
    return it->second;
  }
  return {};
}
}  // namespace baklaga::http::detail

#endif  // BAKLAGA_HTTP_DETAIL_STATUS_CODE_HPP

namespace baklaga::http {
using detail::headers_t;
using detail::method_t;
using detail::status_code_t;

enum class message_t { request, response };

template <message_t Type, bool Mutable = false>
class basic_message {
 public:
  using underlying_t =
      std::conditional_t<Mutable, std::string, std::string_view>;

  basic_message() : method_{}, target_{}, version_{}, headers_{} {}
  basic_message(std::string_view buffer) : basic_message{} { parse(buffer); }

  basic_message& operator=(std::string_view buffer) {
    parse(buffer);
    return *this;
  }

  void parse(std::string_view buffer) {
    auto start_line_end = buffer.find("\r\n");
    if (start_line_end == std::string_view::npos) {
      error_ = std::make_error_code(std::errc::bad_message);
      return;
    }

    auto start_line =
        detail::split_view<3>(buffer.substr(0, start_line_end), " ");
    if constexpr (Type == message_t::request) {
      method_ = detail::to_method(start_line[0]);
      if (method_ == detail::type_npos<method_t>()) {
        error_ = std::make_error_code(std::errc::operation_not_supported);
        return;
      }
      target_ = start_line[1];
      version_ = detail::to_version(start_line[2]);
    } else if constexpr (Type == message_t::response) {
      version_ = detail::to_version(start_line[0]);
      std::from_chars(start_line[1].data(),
                      start_line[1].data() + start_line[1].size(),
                      reinterpret_cast<uint16_t&>(status_code_));
      // status_ = start_line[2];
    }

    if (version_ == detail::type_npos<decltype(version_)>()) {
      error_ = std::make_error_code(std::errc::protocol_not_supported);
      return;
    }

    headers_ = detail::to_headers(
        buffer.substr(start_line_end + 2,
                      buffer.find("\r\n\r\n", start_line_end)));
  }

  std::string build() const {
    std::string result{};

    auto version_str = detail::from_version(version_);
    if constexpr (Type == message_t::request) {
      auto method_str = detail::from_method(method_);

      result =
          std::format("{:s} {:s} {:s}\r\n", method_str, target_, version_str);
    } else if constexpr (Type == message_t::response) {
      auto status_str = detail::from_status_code(status_code_);

      result = std::format("{:s} {:d} {:s}\r\n", version_str,
                           static_cast<uint16_t>(status_code_), status_str);
    }

    for (const auto& [name, content] : headers_) {
      result += std::format("{:s}: {:s}\r\n", name, content);
    }

    return result;
  }

  auto method() const noexcept
    requires(Type == message_t::request)
  {
    return method_;
  }
  auto status_code() const
    requires(Type == message_t::response)
  {
    return status_code_;
  }
  auto target() const noexcept
    requires(Type == message_t::request)
  {
    return target_;
  }
  auto version() const noexcept { return version_; }
  const auto& headers() const noexcept { return headers_; }
  const auto& error() const noexcept { return error_; }

  void method(method_t v) noexcept
    requires(Mutable && Type == message_t::request)
  {
    method_ = v;
  }
  auto status_code(status_code_t v) noexcept
    requires(Mutable && Type == message_t::response)
  {
    status_code_ = v;
  }
  void target(std::string_view v) noexcept
    requires(Mutable && Type == message_t::request)
  {
    target_ = v;
  }
  void version(uint8_t v) noexcept
    requires(Mutable)
  {
    version_ = v;
  }
  void headers(const headers_t& v) noexcept
    requires(Mutable)
  {
    headers_ = v;
  }
  auto& headers() noexcept
    requires(Mutable)
  {
    return headers_;
  }
  operator std::string() const { return build(); }

 private:
  method_t method_;
  status_code_t status_code_;
  underlying_t target_;
  uint8_t version_;
  headers_t headers_;
  std::error_code error_;
};

using request_view = basic_message<message_t::request>;
using request = basic_message<message_t::request, true>;
using response_view = basic_message<message_t::response>;
using response = basic_message<message_t::response, true>;
}  // namespace baklaga::http

#endif  // BAKLAGA_HTTP_MESSAGE__HPP

#endif // BAKLAGA_HTTP_STREAM_HPP

#ifndef BAKLAGA_HTTP_METHODS_HPP
#define BAKLAGA_HTTP_METHODS_HPP

#include <string_view>
#include <system_error>

#ifndef BAKLAGA_HTTP_MESSAGE__HPP
#define BAKLAGA_HTTP_MESSAGE__HPP

#include <cstdint>
#include <format>
#include <string_view>
#include <system_error>
#include <type_traits>

#ifndef BAKLAGA_HTTP_DETAIL_STRING_HPP
#define BAKLAGA_HTTP_DETAIL_STRING_HPP

#include <ranges>
#include <array>
#include <string_view>

namespace baklaga::http::detail {
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
}  // namespace baklaga::http::string

#endif  // BAKLAGA_HTTP_DETAIL_STRING_HPP

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

enum class method_t : uint8_t { get, post, put, delete_ };

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

inline constexpr method_t to_method(std::string_view method_str) {
  if (method_str == "GET") {
    return method_t::get;
  } else if (method_str == "POST") {
    return method_t::post;
  } else if (method_str == "PUT") {
    return method_t::put;
  } else if (method_str == "DELETE") {
    return method_t::delete_;
  }
  return detail::type_npos<method_t>();
}

inline constexpr std::string_view from_method(method_t method) {
  switch (method) {
    case method_t::get:
      return "GET";
    case method_t::post:
      return "POST";
    case method_t::put:
      return "PUT";
    case method_t::delete_:
      return "DELETE";
  }
  return {};
}

inline constexpr uint8_t to_version(std::string_view version_str) noexcept {
  if (version_str == "HTTP/1.0") {
    return 10;
  } else if (version_str == "HTTP/1.1") {
    return 11;
  }
  return type_npos<uint8_t>();
}

inline constexpr std::string_view from_version(uint8_t version) noexcept {
  switch (version) {
    case 10:
      return "HTTP/1.0";
    case 11:
      return "HTTP/1.1";
  }
  return {};
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

#ifndef BAKLAGA_HTTP_DETAIL_STATUS_CODE_HPP
#define BAKLAGA_HTTP_DETAIL_STATUS_CODE_HPP

#include <array>
#include <cstdint>
#include <string_view>

namespace baklaga::http::detail {
enum class status_code_t : uint16_t {
  // Informational
  continue_ = 100,
  switching_protocols = 101,
  processing = 102,
  early_hints = 103,

  // Successful
  ok = 200,
  created = 201,
  accepted = 202,
  non_authoritative_information = 203,
  no_content = 204,
  reset_content = 205,
  partial_content = 206,
  multi_status = 207,
  already_reported = 208,
  im_used = 226,

  // Redirection
  multiple_choices = 300,
  moved_permanently = 301,
  found = 302,
  see_other = 303,
  not_modified = 304,
  use_proxy = 305,
  temporary_redirect = 307,
  permanent_redirect = 308,

  // Client Error
  bad_request = 400,
  unauthorized = 401,
  payment_required = 402,
  forbidden = 403,
  not_found = 404,
  method_not_allowed = 405,
  not_acceptable = 406,
  proxy_authentication_required = 407,
  request_timeout = 408,
  conflict = 409,
  gone = 410,
  length_required = 411,
  precondition_failed = 412,
  payload_too_large = 413,
  uri_too_long = 414,
  unsupported_media_type = 415,
  range_not_satisfiable = 416,
  expectation_failed = 417,
  im_a_teapot = 418,
  misdirected_request = 421,
  unprocessable_entity = 422,
  locked = 423,
  failed_dependency = 424,
  upgrade_required = 426,
  precondition_required = 428,
  too_many_requests = 429,
  request_header_fields_too_large = 431,
  unavailable_for_legal_reasons = 451,

  // Server Error
  internal_server_error = 500,
  not_implemented = 501,
  bad_gateway = 502,
  service_unavailable = 503,
  gateway_timeout = 504,
  http_version_not_supported = 505,
  variant_also_negotiates = 506,
  insufficient_storage = 507,
  loop_detected = 508,
  not_extended = 510,
  network_authentication_required = 511
};

constexpr std::array<std::pair<status_code_t, std::string_view>, 61>
    status_code_map = {
        {{status_code_t::continue_, "Continue"},
         {status_code_t::switching_protocols, "Switching Protocols"},
         {status_code_t::processing, "Processing"},
         {status_code_t::early_hints, "Early Hints"},
         {status_code_t::ok, "OK"},
         {status_code_t::created, "Created"},
         {status_code_t::accepted, "Accepted"},
         {status_code_t::non_authoritative_information,
          "Non Authoritative Information"},
         {status_code_t::no_content, "No Content"},
         {status_code_t::reset_content, "Reset Content"},
         {status_code_t::partial_content, "Partial Content"},
         {status_code_t::multi_status, "Multi-Status"},
         {status_code_t::already_reported, "Already Reported"},
         {status_code_t::im_used, "IM Used"},
         {status_code_t::multiple_choices, "Multiple Choices"},
         {status_code_t::moved_permanently, "Moved Permanently"},
         {status_code_t::found, "Found"},
         {status_code_t::see_other, "See Other"},
         {status_code_t::not_modified, "Not Modified"},
         {status_code_t::use_proxy, "Use Proxy"},
         {status_code_t::temporary_redirect, "Temporary Redirect"},
         {status_code_t::permanent_redirect, "Permanent Redirect"},
         {status_code_t::bad_request, "Bad Request"},
         {status_code_t::unauthorized, "Unauthorized"},
         {status_code_t::payment_required, "Payment Required"},
         {status_code_t::forbidden, "Forbidden"},
         {status_code_t::not_found, "Not Found"},
         {status_code_t::method_not_allowed, "Method Not Allowed"},
         {status_code_t::not_acceptable, "Not Acceptable"},
         {status_code_t::proxy_authentication_required,
          "Proxy Authentication Required"},
         {status_code_t::request_timeout, "Request Timeout"},
         {status_code_t::conflict, "Conflict"},
         {status_code_t::gone, "Gone"},
         {status_code_t::length_required, "Length Required"},
         {status_code_t::precondition_failed, "Precondition Failed"},
         {status_code_t::payload_too_large, "Payload Too Large"},
         {status_code_t::uri_too_long, "URI Too Long"},
         {status_code_t::unsupported_media_type, "Unsupported Media Type"},
         {status_code_t::range_not_satisfiable, "Range Not Satisfiable"},
         {status_code_t::expectation_failed, "Expectation Failed"},
         {status_code_t::im_a_teapot, "I'm a teapot"},
         {status_code_t::misdirected_request, "Misdirected Request"},
         {status_code_t::unprocessable_entity, "Unprocessable Entity"},
         {status_code_t::locked, "Locked"},
         {status_code_t::failed_dependency, "Failed Dependency"},
         {status_code_t::upgrade_required, "Upgrade Required"},
         {status_code_t::precondition_required, "Precondition Required"},
         {status_code_t::too_many_requests, "Too Many Requests"},
         {status_code_t::request_header_fields_too_large,
          "Request Header Fields Too Large"},
         {status_code_t::unavailable_for_legal_reasons,
          "Unavailable For Legal Reasons"},
         {status_code_t::internal_server_error, "Internal Server Error"},
         {status_code_t::not_implemented, "Not Implemented"},
         {status_code_t::bad_gateway, "Bad Gateway"},
         {status_code_t::service_unavailable, "Service Unavailable"},
         {status_code_t::gateway_timeout, "Gateway Timeout"},
         {status_code_t::http_version_not_supported,
          "HTTP Version Not Supported"},
         {status_code_t::variant_also_negotiates, "Variant Also Negotiates"},
         {status_code_t::insufficient_storage, "Insufficient Storage"},
         {status_code_t::loop_detected, "Loop Detected"},
         {status_code_t::not_extended, "Not Extended"},
         {status_code_t::network_authentication_required,
          "Network Authentication Required"}}};

inline constexpr std::string_view from_status_code(status_code_t status_code) {
  auto it = std::find_if(status_code_map.begin(), status_code_map.end(),
                         [status_code](auto& pair) {
                           return pair.first == status_code;
                         });
  if (it != status_code_map.end()) {
    return it->second;
  }
  return {};
}
}  // namespace baklaga::http::detail

#endif  // BAKLAGA_HTTP_DETAIL_STATUS_CODE_HPP

namespace baklaga::http {
using detail::headers_t;
using detail::method_t;
using detail::status_code_t;

enum class message_t { request, response };

template <message_t Type, bool Mutable = false>
class basic_message {
 public:
  using underlying_t =
      std::conditional_t<Mutable, std::string, std::string_view>;

  basic_message() : method_{}, target_{}, version_{}, headers_{} {}
  basic_message(std::string_view buffer) : basic_message{} { parse(buffer); }

  basic_message& operator=(std::string_view buffer) {
    parse(buffer);
    return *this;
  }

  void parse(std::string_view buffer) {
    auto start_line_end = buffer.find("\r\n");
    if (start_line_end == std::string_view::npos) {
      error_ = std::make_error_code(std::errc::bad_message);
      return;
    }

    auto start_line =
        detail::split_view<3>(buffer.substr(0, start_line_end), " ");
    if constexpr (Type == message_t::request) {
      method_ = detail::to_method(start_line[0]);
      if (method_ == detail::type_npos<method_t>()) {
        error_ = std::make_error_code(std::errc::operation_not_supported);
        return;
      }
      target_ = start_line[1];
      version_ = detail::to_version(start_line[2]);
    } else if constexpr (Type == message_t::response) {
      version_ = detail::to_version(start_line[0]);
      std::from_chars(start_line[1].data(),
                      start_line[1].data() + start_line[1].size(),
                      reinterpret_cast<uint16_t&>(status_code_));
      // status_ = start_line[2];
    }

    if (version_ == detail::type_npos<decltype(version_)>()) {
      error_ = std::make_error_code(std::errc::protocol_not_supported);
      return;
    }

    headers_ = detail::to_headers(
        buffer.substr(start_line_end + 2,
                      buffer.find("\r\n\r\n", start_line_end)));
  }

  std::string build() const {
    std::string result{};

    auto version_str = detail::from_version(version_);
    if constexpr (Type == message_t::request) {
      auto method_str = detail::from_method(method_);

      result =
          std::format("{:s} {:s} {:s}\r\n", method_str, target_, version_str);
    } else if constexpr (Type == message_t::response) {
      auto status_str = detail::from_status_code(status_code_);

      result = std::format("{:s} {:d} {:s}\r\n", version_str,
                           static_cast<uint16_t>(status_code_), status_str);
    }

    for (const auto& [name, content] : headers_) {
      result += std::format("{:s}: {:s}\r\n", name, content);
    }

    return result;
  }

  auto method() const noexcept
    requires(Type == message_t::request)
  {
    return method_;
  }
  auto status_code() const
    requires(Type == message_t::response)
  {
    return status_code_;
  }
  auto target() const noexcept
    requires(Type == message_t::request)
  {
    return target_;
  }
  auto version() const noexcept { return version_; }
  const auto& headers() const noexcept { return headers_; }
  const auto& error() const noexcept { return error_; }

  void method(method_t v) noexcept
    requires(Mutable && Type == message_t::request)
  {
    method_ = v;
  }
  auto status_code(status_code_t v) noexcept
    requires(Mutable && Type == message_t::response)
  {
    status_code_ = v;
  }
  void target(std::string_view v) noexcept
    requires(Mutable && Type == message_t::request)
  {
    target_ = v;
  }
  void version(uint8_t v) noexcept
    requires(Mutable)
  {
    version_ = v;
  }
  void headers(const headers_t& v) noexcept
    requires(Mutable)
  {
    headers_ = v;
  }
  auto& headers() noexcept
    requires(Mutable)
  {
    return headers_;
  }
  operator std::string() const { return build(); }

 private:
  method_t method_;
  status_code_t status_code_;
  underlying_t target_;
  uint8_t version_;
  headers_t headers_;
  std::error_code error_;
};

using request_view = basic_message<message_t::request>;
using request = basic_message<message_t::request, true>;
using response_view = basic_message<message_t::response>;
using response = basic_message<message_t::response, true>;
}  // namespace baklaga::http

#endif  // BAKLAGA_HTTP_MESSAGE__HPP

namespace baklaga::http {
template <class Socket>
std::tuple<http::response, std::error_code> get(std::string_view uri,
                                                std::string_view body = {},
                                                const headers_t& headers = {}) {
  return {};
}
}  // namespace baklaga::http

#endif  // BAKLAGA_HTTP_METHODS_HPP

#endif // BAKLAGA_HTTP_HPP
