#ifndef BACKEND_APIGATEWAY_SRC_GATEWAYSERVER_GATEWAYSERVER_H_
#define BACKEND_APIGATEWAY_SRC_GATEWAYSERVER_GATEWAYSERVER_H_

#include <crow.h>
#include <ixwebsocket/IXWebSocket.h>

#include <string>

#include "JwtUtils.h"
#include "ScopedRequestsTimer.h"
#include "proxyclient.h"
#include "ratelimiter.h"

class GatewayServer {
 public:
  explicit GatewayServer(int port);
  void run();

 private:
  crow::SimpleApp app_;
  int             port_;

  RateLimiter rateLimiter_;
  ProxyClient authProxy_;
  ProxyClient chatProxy_;
  ProxyClient messageProxy_;
  ProxyClient notificationProxy_;

  std::unique_ptr<prometheus::Exposer>  exposer_;
  std::shared_ptr<prometheus::Registry> registry_;

  prometheus::Family<prometheus::Counter>& request_counter_family_;
  prometheus::Histogram&                   request_latency_;

  void handleProxyRequest(const crow::request& req,
                          crow::response&      res,
                          ProxyClient&         proxy,
                          const std::string&   path,
                          bool                 requireAuth);

  void registerRoute(const std::string& basePath, ProxyClient& proxy, bool requireAuth = true);

  void registerRoutes();
  void registerHealthCheck();
  void registerWebSocketRoutes();

  bool        checkRateLimit(const crow::request& req, crow::response& res);
  std::string extractToken(const crow::request& req) const;
};

#endif  // BACKEND_APIGATEWAY_SRC_GATEWAYSERVER_GATEWAYSERVER_H_
