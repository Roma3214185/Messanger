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
#include "ProxyRepository.h"
#include "GatewayMetrics.h"
#include "ratelimiter.h"

class ICacheService;

class GatewayServer {
 public:
  GatewayServer(crow::SimpleApp& app, ICacheService* cache, IConfigProvider* = &ProdConfigProvider::instance());
  void run();

 private:
  crow::SimpleApp& app_;
  IConfigProvider* provider_;
  ICacheService* cache_;
  Logger logger_;
  ProxyRepository proxy_repository_;
  GatewayMetrics metrics_;
  RateLimiter rate_limiter_;


  void handleProxyRequest(const crow::request& req,
                          crow::response&      res,
                          ProxyClient&         proxy,
                          const std::string&   path,
                          bool                 requireAuth);

  void registerRoute(const std::string& basePath, ProxyClient& proxy, bool requireAuth = true);
  bool checkAuth(const crow::request& req, bool requireAuth);
  std::string extractIP(const crow::request& req);  //TODO: remove from here
  void sendResponde(crow::response& res, const RequestDTO&, int res_code, const std::string& message, bool hitKey = false);

  void registerRoutes();
  void registerHealthCheck();
  void registerWebSocketRoutes();

  std::optional<nlohmann::json> checkCache(std::string key);
  void saveInCache(const crow::request& req, std::string key, std::string value, std::chrono::milliseconds ttl = std::chrono::milliseconds(60));

  bool        checkRateLimit(const crow::request& req);
  std::string extractToken(const crow::request& req) const;
};

#endif  // BACKEND_APIGATEWAY_SRC_GATEWAYSERVER_GATEWAYSERVER_H_
