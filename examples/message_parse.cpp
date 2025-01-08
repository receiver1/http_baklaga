#include <baklaga/http/message.hpp>
#include <iostream>
#include <string_view>

int main(int argc, char* argv[]) {
  using namespace baklaga;

  std::string_view request_str{
      "GET /test11 HTTP/1.1\r\n"
      "User-Agent: baklaga11\r\n"
      "Accept-Language: ru-RU\r\n"
      "Content-Length: 4\r\n\r\n"};

  // Pasring request
  http::request_view request{request_str};

  std::cout << "\n> Request:\n";
  std::cout << http::detail::from_method(request.method()) << " "
            << request.target() << " "
            << http::detail::from_version(request.version()) << std::endl;
  for (const auto& [name, content] : request.headers()) {
    std::cout << name << ": " << content << std::endl;
  }

  // Building response
  baklaga::http::response response{};
  response.version(10);
  response.status_code(http::status_code_t::ok);
  response.headers().emplace("User-Agent", "baklaga11");
  response.headers().emplace("Language", "ru-RU");

  std::cout << "\n> Response:\n" << response.build() << std::endl;

  return 0;
}