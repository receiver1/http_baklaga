#ifndef BAKLAGA_HTTP_METHOD_HPP
#define BAKLAGA_HTTP_METHOD_HPP

#include <string_view>
#include <system_error>

#include "baklaga/http/message.hpp"

namespace baklaga::http {
template <class Socket>
std::tuple<http::response, std::error_code> get(std::string_view uri,
                                                std::string_view body = {},
                                                const headers_t& headers = {}) {
  return {};
}
}  // namespace baklaga::http

#endif  // BAKLAGA_HTTP_METHOD_HPP
