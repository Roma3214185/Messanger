#include "gatewayserver.h"
#include "ProdConfigProvider.h"
#include "RedisCache.h"
#include "GatewayMetrics.h"
#include "RealHttpClient.h"
#include "middlewares/Middlewares.h"
#include "JWTVerifier.h"
#include "ratelimiter.h"
#include "ThreadPool.h"
#include "GatewayMetrics.h"
#include "RabbitMQClient.h"
#include "mocks/MockRabitMQClient.h" //TODO: remove mocks from cmake and here

const std::string   kKeysDir       = "/Users/roma/QtProjects/Chat/Backend/shared_keys/";
const std::string   kPublicKeyFile = kKeysDir + "public_key.pem";
const std::string   kIssuer        = "auth_service";

int main() {
  init_logger("Gateway");
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
  MockRabitMQClient rabbit_client;
  GatewayServer server(app, &client, &cache, &pool, &provider, &rabbit_client);
  server.registerRoutes();
  server.run();
  return 0;
}


