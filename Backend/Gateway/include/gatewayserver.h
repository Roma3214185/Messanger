#ifndef BACKEND_APIGATEWAY_SRC_GATEWAYSERVER_GATEWAYSERVER_H_
#define BACKEND_APIGATEWAY_SRC_GATEWAYSERVER_GATEWAYSERVER_H_

#include <string>

#include <crow.h>
#include <ixwebsocket/IXWebSocket.h>

#include "ProdConfigProvider.h"
#include "proxyclient.h"
#include "middlewares/Middlewares.h"
#include "interfaces/IMetrics.h"

class ICacheService;
class IThreadPool;
class IClient;
class IMetrics;

using GatewayApp = crow::App<
    LoggingMiddleware,
    RateLimitMiddleware,
    MetricsMiddleware,
    AuthMiddleware,
    CacheMiddleware
    >;

class GatewayServer {
 public:
  GatewayServer(GatewayApp& app, IClient* client,
                 IThreadPool* pool, IConfigProvider* provider);
  void run();
  void registerRoutes();

 protected:
  virtual void sendResponse(crow::response& res, int res_code, const std::string& message);

 private:
  GatewayApp& app_;
  IConfigProvider* provider_;
  ICacheService* cache_;
  ProxyClient proxy_;
  IThreadPool* pool_;

  void handleProxyRequest(const crow::request&, crow::response&, int service_port, const std::string& path);
  void registerRoute(const std::string& basePath, int proxy);
  void registerHealthCheck();
  void registerWebSocketRoutes();
};

#endif  // BACKEND_APIGATEWAY_SRC_GATEWAYSERVER_GATEWAYSERVER_H_
