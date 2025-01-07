#ifndef BAKLAGA_HTTP_RESPONSE_HPP
#define BAKLAGA_HTTP_RESPONSE_HPP

#include <cstdint>
#include <string_view>
#include <system_error>

#include "baklaga/http/detail/message.hpp"

namespace baklaga::http {
using detail::headers_t;

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

class response {
 public:
  response(std::string_view buffer) : version_{}, status_code_{}, headers_{} {
    parse_(buffer);
  }

  response& operator=(std::string_view&& buffer) {
    parse_(buffer);
    return *this;
  }

  auto version() const { return version_; }
  auto status_code() const { return status_code_; }
  auto& headers() { return headers_; }
  const auto& error() const { return error_; }

 private:
  void parse_(std::string_view buffer);

  uint8_t version_;
  status_code_t status_code_;
  std::string_view status_;
  headers_t headers_;
  std::error_code error_;
};
}  // namespace baklaga::http

#include "baklaga/http/impl/response.ipp"

#endif  // BAKLAGA_HTTP_RESPONSE_HPP
