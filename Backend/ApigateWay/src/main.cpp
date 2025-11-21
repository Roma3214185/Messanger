#include "gatewayserver.h"
#include "ProdConfigProvider.h"
#include "RedisCache.h"
#include "ProxyRepository.h"
#include "GatewayMetrics.h"
#include "proxyclient.h"

int main() {
  ProdConfigProvider provider;
  crow::SimpleApp app;
  RedisCache& cache = RedisCache::instance();
  ProxyClient proxy;
  GatewayServer server(app, &cache, &proxy, &provider);
  server.run();
  return 0;
}


