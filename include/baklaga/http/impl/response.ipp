#ifndef BAKLAGA_HTTP_IMPL_RESPONSE_IPP
#define BAKLAGA_HTTP_IMPL_RESPONSE_IPP

#include <charconv>

#include "baklaga/http/detail/message.hpp"

void baklaga::http::response::parse_(std::string_view buffer) {
  auto start_line_end = buffer.find("\r\n");
  if (start_line_end == std::string_view::npos) {
    error_ = std::make_error_code(std::errc::bad_message);
    return;
  }

  auto start_line =
      detail::split_view<3>(buffer.substr(0, start_line_end), " ");
  version_ = detail::to_version(start_line[0]);
  if (version_ == detail::type_npos<uint8_t>()) {
    error_ = std::make_error_code(std::errc::protocol_not_supported);
    return;
  }
  std::from_chars(start_line[1].data(),
                  start_line[1].data() + start_line[1].size(),
                  reinterpret_cast<uint16_t&>(status_code_));
  status_ = start_line[2];

  headers_ = detail::to_headers(buffer.substr(
      start_line_end + 2, buffer.find("\r\n\r\n", start_line_end)));
}

#endif  // BAKLAGA_HTTP_IMPL_RESPONSE_IPP