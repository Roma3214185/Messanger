#ifndef BACKEND_APIGATEWAY_SRC_GATEWAYSERVER_GATEWAYSERVER_H_
#define BACKEND_APIGATEWAY_SRC_GATEWAYSERVER_GATEWAYSERVER_H_

#include <crow.h>
#include <ixwebsocket/IXWebSocket.h>

#include <string>

#include "interfaces/IMetrics.h"
#include "middlewares/Middlewares.h"
#include "proxyclient.h"

class IEventBus;
class ICacheService;
class IThreadPool;
class IClient;
class IMetrics;

using GatewayApp =
    crow::App<LoggingMiddleware, RateLimitMiddleware, MetricsMiddleware, AuthMiddleware, CacheMiddleware>;

class GatewayServer {
 public:
  GatewayServer(GatewayApp &app, IClient *client, ICacheService *cache, IThreadPool *pool, IEventBus *queue);
  void run();
  void registerRoutes();

 protected:
  virtual void sendResponse(crow::response &res, int res_code, const std::string &message);

 private:
  GatewayApp &app_;
  ICacheService *cache_;
  ProxyClient proxy_;
  IThreadPool *pool_;
  IEventBus *queue_;

  void handleProxyRequest(const crow::request &, crow::response &, const int service_port, const std::string &path);
  void handlePostRequest(const crow::request &req, crow::response &res, const int port, const std::string &path);
  void registerRequestRoute();
  void registerRoute(const std::string &basePath, int proxy);
  void registerHealthCheck();
  void registerWebSocketRoutes();
  void subscribeOnNewRequest();  // TODO: worker class ?
};

#endif  // BACKEND_APIGATEWAY_SRC_GATEWAYSERVER_GATEWAYSERVER_H_
