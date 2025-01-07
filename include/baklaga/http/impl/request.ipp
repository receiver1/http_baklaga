#ifndef BAKLAGA_HTTP_IMPL_REQUEST_IPP
#define BAKLAGA_HTTP_IMPL_REQUEST_IPP

void baklaga::http::request::parse_(std::string_view buffer) {
  auto start_line_end = buffer.find("\r\n");
  if (start_line_end == std::string_view::npos) {
    error_ = std::make_error_code(std::errc::bad_message);
    return;
  }

  auto start_line =
      detail::split_view<3>(buffer.substr(0, start_line_end), " ");
  method_ = detail::to_method(start_line[0]);
  if (method_ == static_cast<method_t>(-1)) {
    error_ = std::make_error_code(std::errc::operation_not_supported);
    return;
  }
  target_ = start_line[1];
  version_ = detail::to_version(start_line[2]);
  if (version_ == std::numeric_limits<uint8_t>::max()) {
    error_ = std::make_error_code(std::errc::protocol_not_supported);
    return;
  }

  headers_ = detail::to_headers(buffer.substr(
      start_line_end + 2, buffer.find("\r\n\r\n", start_line_end)));
}

#endif  // BAKLAGA_HTTP_IMPL_REQUEST_IPP