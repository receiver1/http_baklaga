#include <baklaga/http/uri.hpp>
#include <baklaga/http/uri_encode.hpp>
#include <iostream>
#include <string_view>

int main(int argc, char* argv[]) {
  using namespace baklaga;

  std::string_view uri_str{
      "https://receiver:annihilatorq@example.com:443/"
      "index.php?param1=value1&param2=value2#fragment"};

  // Pasring URI
  http::uri_view uri{uri_str};

  std::cout << uri.scheme() << std::endl;
  std::cout << uri.authority().username() << std::endl;
  std::cout << uri.authority().password() << std::endl;
  std::cout << uri.authority().hostname() << std::endl;
  std::cout << uri.authority().port() << std::endl;
  std::cout << uri.path() << std::endl;
  for (auto& q : uri.query()) {
    std::cout << q.first << "=" << q.second << std::endl;
  }
  std::cout << uri.fragment() << std::endl;

  // Building URI
  http::uri uri2{};
  uri2.scheme("https");
  uri2.authority().username("receiver");
  uri2.authority().password("annihilatorq");
  uri2.authority().hostname("example.com");
  uri2.authority().port(443);
  uri2.path("/index.php");
  uri2.query().emplace("param1", "value1");
  uri2.query().emplace("param2", "value2");
  uri2.fragment("fragment");

  std::cout << "Builded: " << http::uri_encode(uri2.build())
            << std::endl;

  return 0;
}