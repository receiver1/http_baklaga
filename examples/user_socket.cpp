#include <baklaga/http/stream.hpp>
#include <iostream>
#include <string_view>
#include <system_error>

namespace tcp {
class socket {
 public:
  void open(std::error_code& ec) {}
  void connect(std::string_view host, std::string_view port, std::error_code& error) {}
  size_t read(std::span<uint8_t> buffer, std::error_code& error) { return 0; }
  size_t write(const std::span<uint8_t> buffer, std::error_code& error) { return 0; }
  void shutdown(std::error_code& error) {}
  void close(std::error_code& error) {}
};
}  // namespace tcp

int main(int argc, char* argv[]) {
  using namespace baklaga;

  http::stream<tcp::socket> http{};
 
  // auto [res, err] = http::get<tcp::socket>("http://example.com/get", "",
  //                                          {
  //                                              {"User-Agent", "baklaga"},
  //                                              {"Accept-Language", "ru-RU"},
  //                                          });
  // if (!err && res.status_code() == http::status_code_t::ok) {
  //   std::cout << res.body() << std::endl;
  // }

  return 0;
}