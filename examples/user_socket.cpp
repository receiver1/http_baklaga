#include <baklaga/http/methods.hpp>
#include <baklaga/http/stream.hpp>
#include <iostream>

namespace tcp {
class socket {
 public:
  void open() {}
  void connect() {}
  void read() {}
  void write() {}
  void shutdown() {}
  void close() {}
};
}  // namespace tcp

int main(int argc, char* argv[]) {
  using namespace baklaga;

  auto [res, err] = http::get<tcp::socket>("http://example.com/get", "",
                                           {
                                               {"User-Agent", "baklaga"},
                                               {"Accept-Language", "ru-RU"},
                                           });
  if (!err && res.status_code() == http::status_code_t::ok) {
    std::cout << res.body() << std::endl;
  }

  return 0;
}