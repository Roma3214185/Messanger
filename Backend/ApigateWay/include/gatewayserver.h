#ifndef BACKEND_APIGATEWAY_SRC_GATEWAYSERVER_GATEWAYSERVER_H_
#define BACKEND_APIGATEWAY_SRC_GATEWAYSERVER_GATEWAYSERVER_H_

#include <string>

#include <crow.h>
#include <ixwebsocket/IXWebSocket.h>

#include "ProdConfigProvider.h"
#include "proxyclient.h"
#include "middlewares/AuthMiddleware.h"
#include "middlewares/CacheMiddleware.h"
#include "middlewares/LoggingMiddleware.h"
#include "middlewares/RateLimitMiddleware.h"
#include "interfaces/IMetrics.h"

class ICacheService;
class IThreadPool;
class IClient;
class IMetrics;

using GatewayApp = crow::App<
    LoggingMiddleware,
    RateLimitMiddleware,
    AuthMiddleware,
    CacheMiddleware
    >;

class GatewayServer {
 public:
  GatewayServer(GatewayApp& app, ICacheService* cache, IClient* client,
                 IThreadPool* pool, IMetrics* metrics, IConfigProvider* provider);
  void run();
  void registerRoutes();

 protected:
  virtual void sendResponse(crow::response& res, const RequestDTO&, int res_code, const std::string& message, bool hitKey = false);

 private:
  GatewayApp& app_;
  IConfigProvider* provider_;
  ICacheService* cache_;
  ProxyClient proxy_;
  IMetrics* metrics_;
  IThreadPool* pool_;

  void handleProxyRequest(const crow::request&, crow::response&, int service_port, const std::string& path, bool requireAuth);
  void registerRoute(const std::string& basePath, int proxy, bool requireAuth = true);
  void registerHealthCheck();
  void registerWebSocketRoutes();
  void saveInCache(const crow::request& req, std::string key, std::string value, std::chrono::milliseconds ttl = std::chrono::milliseconds(60));
};

#endif  // BACKEND_APIGATEWAY_SRC_GATEWAYSERVER_GATEWAYSERVER_H_
