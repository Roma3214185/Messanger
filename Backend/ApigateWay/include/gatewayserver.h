#ifndef BACKEND_APIGATEWAY_SRC_GATEWAYSERVER_GATEWAYSERVER_H_
#define BACKEND_APIGATEWAY_SRC_GATEWAYSERVER_GATEWAYSERVER_H_

#include <crow.h>
#include <ixwebsocket/IXWebSocket.h>

#include <string>

#include "JwtUtils.h"
#include "ScopedRequestsTimer.h"
#include "proxyclient.h"
#include "ratelimiter.h"
#include "ProdConfigProvider.h"
#include "Logger.h"
#include "GatewayMetrics.h"
#include "ratelimiter.h"
#include "proxyclient.h"

class ICacheService;

class GatewayServer {
 public:
  GatewayServer(crow::SimpleApp& app, ICacheService* cache, IClient* client, IConfigProvider* provider);
  void run();

 protected:
  virtual void sendResponse(crow::response& res, const RequestDTO&, int res_code, const std::string& message, bool hitKey = false);
  std::string extractToken(const crow::request&) const; //TODO: remove from here
  std::string extractIP(const crow::request& req) const;


 private:
  crow::SimpleApp& app_;
  IConfigProvider* provider_;
  ICacheService* cache_;
  Logger logger_;
  ProxyClient proxy_;
  GatewayMetrics metrics_;
  RateLimiter rate_limiter_;


  void handleProxyRequest(const crow::request&, crow::response&, int service_port, const std::string& path, bool requireAuth);
  void registerRoute(const std::string& basePath, int proxy, bool requireAuth = true);

  void registerRoutes();
  void registerHealthCheck();
  void registerWebSocketRoutes();

  std::optional<nlohmann::json> checkCache(std::string key);
  void saveInCache(const crow::request& req, std::string key, std::string value, std::chrono::milliseconds ttl = std::chrono::milliseconds(60));

  virtual bool checkRateLimit(const crow::request&);
  virtual bool checkAuth(const crow::request& req, bool requireAuth);
};

#endif  // BACKEND_APIGATEWAY_SRC_GATEWAYSERVER_GATEWAYSERVER_H_
