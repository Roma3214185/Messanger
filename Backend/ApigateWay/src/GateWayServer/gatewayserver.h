#ifndef BACKEND_APIGATEWAY_SRC_GATEWAYSERVER_GATEWAYSERVER_H_
#define BACKEND_APIGATEWAY_SRC_GATEWAYSERVER_GATEWAYSERVER_H_

#include <crow.h>
#include <../external/IXWebSocket/ixwebsocket/IXWebSocket.h>

#include <string>

#include "Headers/AuthVerifier.h"
#include "Headers/ratelimiter.h"
#include "ProxyClient/proxyclient.h"

class GatewayServer {
 public:
  explicit GatewayServer(const int& port);
  void run();

 private:
  crow::SimpleApp app_;
  int port_;

  RateLimiter rateLimiter_;
  AuthVerifier authVerifier_;
  ProxyClient authProxy_;
  ProxyClient chatProxy_;
  ProxyClient messageProxy_;
  ProxyClient notificationProxy_;

  std::unordered_map<crow::websocket::connection*, std::shared_ptr<ix::WebSocket>> client_to_backend;

  void registerRoutes();
  void registrerHealthCheck();
  void registerNotificationRoutes();
  void registerUserRoutes();
  void registerMessagesRoutes();
  void registerChatRoutes();
  void registerAuthRoutes();
  void registerWebSocketRoutes();

  bool checkRateLimit(const crow::request& req, crow::response& res);
  std::string getMethod(const crow::HTTPMethod& method) const;
  std::string extractToken(const crow::request& req) const;
};

#endif  // BACKEND_APIGATEWAY_SRC_GATEWAYSERVER_GATEWAYSERVER_H_
