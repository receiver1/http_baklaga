#include <baklaga/http/methods.hpp>
#include <baklaga/http/stream.hpp>

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

  auto response = http::get<tcp::socket>("http://example.com/get", "",
                                         {
                                             {"User-Agent", "baklaga"},
                                             {"Accept-Language", "ru-RU"},
                                         })
                      .response();
  if (response.status_code() == http::status_code_t::ok) {
    std::cout << response.body() << std::endl;
  }

  return 0;
}