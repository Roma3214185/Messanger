#include "gatewayserver.h"
#include "ProdConfigProvider.h"
#include "RedisCache.h"
#include "GatewayMetrics.h"
#include "RealHttpClient.h"

int main() {
  ProdConfigProvider provider;
  crow::SimpleApp app;
  RedisCache& cache = RedisCache::instance();
  RealHttpClient client;
  GatewayServer server(app, &cache, &client, &provider);
  server.run();
  return 0;
}


