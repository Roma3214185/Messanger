#include "gatewayserver.h"

#include <chrono>
#include <cstdlib>
#include <iostream>

#include "Debug_profiling.h"
#include "ScopedRequestsTimer.h"
#include "websocketbridge.h"
#include "interfaces/ICacheService.h"
#include "middlewares/AuthMiddleware.h"
#include "middlewares/CacheMiddleware.h"
#include "middlewares/LoggingMiddleware.h"
#include "middlewares/RateLimitMiddleware.h"
#include "interfaces/IThreadPool.h"
#include "interfaces/IMetrics.h"
#include "MetricsTracker.h"

using json = nlohmann::json;
using namespace std::chrono_literals;
using std::string;

namespace {

std::string makeCacheKey(const crow::request& req) {
  return "cache:" + crow::method_name(req.method) + ":" + req.url;
}

}  // namespace

GatewayServer::GatewayServer(GatewayApp& app,
                             ICacheService* cache, IClient* client, IThreadPool* pool, IMetrics* metrics, IConfigProvider* provider)
    : app_(app)
    , provider_(provider)
    , metrics_(metrics)
    , proxy_(client)
    , cache_(cache)
    , pool_(pool) {
}

void GatewayServer::run() {
  LOG_INFO("Starting API Gateway on port {}", provider_->ports().apigateService);
  app_.port(provider_->ports().apigateService).multithreaded().run();
}

void GatewayServer::sendResponse(crow::response& res, const RequestDTO& request_info, int res_code, const std::string& message, bool hitKey) {
  res.code = res_code;
  res.write(message);
  res.end();

  metrics_->requestEnded(request_info.path, res_code, hitKey);
}

void GatewayServer::registerRoutes() {
  registerRoute("/auth", provider_->ports().authService, false);
  registerRoute("/users", provider_->ports().authService, false);
  registerRoute("/chats", provider_->ports().chatService, false);
  registerRoute("/messages", provider_->ports().messageService, false);
  registerRoute("/notification", provider_->ports().notificationService, false);
  registerHealthCheck();
  registerWebSocketRoutes();
}

void GatewayServer::registerRoute(const std::string& basePath,
                                  int port,
                                  bool               requireAuth) {
  app_.route_dynamic(basePath + "/<path>")
      .methods("GET"_method,
               "POST"_method)([this, port, basePath, requireAuth](
                                  const crow::request& req, crow::response& res, std::string path) {
        pool_->enqueue([this, req = std::move(req), &res, port, basePath, requireAuth, path]() mutable {
          handleProxyRequest(req, res, port, basePath + "/" + path, requireAuth);
          //TODO: i need res.end() here or in handleProxyRequest
        });
      });

  app_.route_dynamic(basePath).methods("GET"_method, "POST"_method)(
      [this, port, basePath, requireAuth](const crow::request& req, crow::response& res) {
        pool_->enqueue([this, req = std::move(req), &res, port, basePath, requireAuth]() mutable {
         handleProxyRequest(req, res, port, basePath, requireAuth);
        });
      });
}

void GatewayServer::saveInCache(const crow::request& req, std::string key, std::string value, std::chrono::milliseconds ttl) {
  if(req.method != crow::HTTPMethod::Get) return;
  cache_->set(key, value, ttl);
}

void GatewayServer::handleProxyRequest(const crow::request& req,
                                       crow::response&      res,
                                       int port,
                                       const std::string&   path,
                                       bool                 requireAuth) {
  auto tracker = metrics_->getTracker(path);
  PROFILE_SCOPE(path.c_str());

  RequestDTO request_info;
  request_info.method = crow::method_name(req.method);
  request_info.path = path;

  auto result = proxy_.forward(req, request_info, port);

  auto key = makeCacheKey(req);
  saveInCache(req, key, result.second);
  sendResponse(res, request_info, result.first, result.second);
}

void GatewayServer::registerWebSocketRoutes() {
  auto wsBridge = std::make_shared<WebSocketBridge>("ws://127.0.0.1:8086/ws");

  CROW_WEBSOCKET_ROUTE(app_, "/ws")
      .onopen([wsBridge, this](crow::websocket::connection& client) {
        wsBridge->onClientConnect(client);
        metrics_->userConnected();
      })
      .onmessage([wsBridge, this](crow::websocket::connection& client, const std::string& data, bool) {
        wsBridge->onClientMessage(client, data);
        metrics_->newMessage(client.get_remote_ip());
      })
      .onclose([wsBridge, this](crow::websocket::connection& client,
                          const std::string&           reason,
                          uint16_t code) {
        wsBridge->onClientClose(client, reason, code);
        metrics_->userDisconnected();
      });
}

void GatewayServer::registerHealthCheck() {
  CROW_ROUTE(app_, "/healthz")([](const crow::request&, crow::response& res) {
    json info = {{"status", "ok"},
                 {"timestamp",
                  std::chrono::duration_cast<std::chrono::milliseconds>(
                      std::chrono::system_clock::now().time_since_epoch())
                      .count()}};
    res.set_header("Content-Type", "application/json");
    res.write(info.dump());
    res.end();
  });
}
