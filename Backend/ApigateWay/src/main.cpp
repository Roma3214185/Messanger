#include "gatewayserver.h"

int main() {
  const int     port = std::stoi(std::getenv("PORT") ? std::getenv("PORT") : "8084");
  GatewayServer server(port);
  server.run();
  return 0;
}
