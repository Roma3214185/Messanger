#include "gatewayserver.h"
#include "ProdConfigProvider.h"

int main() {
  ProdConfigProvider provider;
  GatewayServer server(provider.ports().apigateService);
  server.run();
  return 0;
}
