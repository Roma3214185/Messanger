#include "gatewayserver.h"
#include "ProdConfigProvider.h"
#include "RedisCache.h"
#include "GatewayMetrics.h"
#include "RealHttpClient.h"
#include "middlewares/AuthMiddleware.h"
#include "middlewares/CacheMiddleware.h"
#include "middlewares/LoggingMiddleware.h"
#include "middlewares/RateLimitMiddleware.h"
#include "middlewares/MetricsMiddleware.h"
#include "JWTVerifier.h"
#include "ratelimiter.h"
#include "ThreadPool.h"
#include "GatewayMetrics.h"

const std::string   kKeysDir       = "/Users/roma/QtProjects/Chat/Backend/shared_keys/";
const std::string   kPublicKeyFile = kKeysDir + "public_key.pem";
const std::string   kIssuer        = "auth_service";

int main() {
  ProdConfigProvider provider;

  JWTVerifier verifier(kPublicKeyFile, kIssuer);
  RedisCache& cache = RedisCache::instance();
  RateLimiter rate_limiter;
  GatewayMetrics metrics(provider.ports().metrics);

  GatewayApp app;
  app.get_middleware<AuthMiddleware>().verifier_ = &verifier;
  app.get_middleware<MetricsMiddleware>().metrics_ = &metrics;
  app.get_middleware<CacheMiddleware>().cache_ = &cache;
  app.get_middleware<LoggingMiddleware>();
  app.get_middleware<RateLimitMiddleware>().rate_limiter_ = &rate_limiter;

  RealHttpClient client;
  ThreadPool pool(8);
  GatewayServer server(app, &cache, &client, &pool, &provider);
  server.registerRoutes();
  server.run();
  return 0;
}


