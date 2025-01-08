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
