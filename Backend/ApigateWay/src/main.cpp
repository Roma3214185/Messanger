#include "gatewayserver.h"

#include "ports.h"

int main() {
  GatewayServer server(ports::ApigateServicePort);
  server.run();
  return 0;
}
