#ifndef BACKEND_APIGATEWAY_SRC_GATEWAYSERVER_GATEWAYSERVER_H_
#define BACKEND_APIGATEWAY_SRC_GATEWAYSERVER_GATEWAYSERVER_H_

#include <crow.h>
#include <string>

#include "middlewares/Middlewares.h"

class GatewayController;

using GatewayApp =
    crow::App<LoggingMiddleware, RateLimitMiddleware, MetricsMiddleware, AuthMiddleware, CacheMiddleware>;

class GatewayServer {
 public:
  GatewayServer(GatewayApp &app, GatewayController *controller);
  void run();
  void registerRoutes();

 private:
  GatewayApp &app_;
  GatewayController *controller_;

  void registerRequestRoute();
  void registerRoute(const std::string &basePath, int proxy);
  void registerHealthCheck();
  void registerWebSocketRoutes();
};

#endif  // BACKEND_APIGATEWAY_SRC_GATEWAYSERVER_GATEWAYSERVER_H_
