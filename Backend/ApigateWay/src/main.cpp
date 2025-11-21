#include "gatewayserver.h"
#include "ProdConfigProvider.h"
#include "RedisCache.h"
#include "ProxyRepository.h"
#include "GatewayMetrics.h"

int main() {
  ProdConfigProvider provider;
  crow::SimpleApp app;
  RedisCache& cache = RedisCache::instance();
  //ProxyRepository repository(&provider);
  GatewayServer server(app, &cache, &provider);
  server.run();
  return 0;
}


