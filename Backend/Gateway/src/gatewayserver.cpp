#include "gatewayserver.h"

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <fmt/format.h>
#include <uuid/uuid.h>

#include "Debug_profiling.h"
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
using std::string;

namespace {

inline long long getCurrentTime() {
  using namespace std::chrono;
  return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

inline std::string generateRequestID() {
  uuid_t uuid;
  uuid_generate_random(uuid);
  char str[37];
  uuid_unparse(uuid, str);
  return std::string(str);
}

}  // namespace

GatewayServer::GatewayServer(GatewayApp& app, IClient* client, IThreadPool* pool, IConfigProvider* provider)
    : app_(app)
    , provider_(provider)
    , proxy_(client)
    , pool_(pool) {
}

void GatewayServer::run() {
  LOG_INFO("Starting API Gateway on port {}", provider_->ports().apigateService);
  app_.port(provider_->ports().apigateService).multithreaded().run();
}

void GatewayServer::sendResponse(crow::response& res, int res_code, const std::string& message) {
  res.code = res_code;
  res.write(message);
  res.end();
}

void GatewayServer::registerRoutes() {
  registerRoute("/auth", provider_->ports().authService);
  registerRoute("/users", provider_->ports().authService);
  registerRoute("/chats", provider_->ports().chatService);
  registerRoute("/messages", provider_->ports().messageService);
  registerRoute("/notification", provider_->ports().notificationService);
  registerRequestRoute();
  registerHealthCheck();
  registerWebSocketRoutes();
}

void GatewayServer::registerRequestRoute() {
  CROW_ROUTE(app_, "/request/<string>/status")
  .methods("GET"_method)
      ([this](const crow::request& req, crow::response& res, std::string task_id) {

        std::optional<std::string> status = cache_->get("request:" + task_id);

        if (!status) {
          sendResponse(res, 404, "{\"error\": \"request not found\"}");
        } else {
          sendResponse(res, 200, status.value());
        }
      });
}

void GatewayServer::registerRoute(const std::string& basePath,
                                  int port) {
  app_.route_dynamic(basePath + "/<path>")
      .methods("GET"_method,
               "POST"_method)([this, port, basePath](
                                  const crow::request& req, crow::response& res, std::string path) {
        //pool_->enqueue([this, req = std::move(req), &res, port, basePath, path]() mutable {
          handleProxyRequest(req, res, port, basePath + "/" + path);
          //TODO: res.end() here or in handleProxyRequest
          //TODO: make async
       // });
      });

  app_.route_dynamic(basePath).methods("GET"_method, "POST"_method)(
      [this, port, basePath](const crow::request& req, crow::response& res) {
        //pool_->enqueue([this, req = std::move(req), &res, port, basePath]() mutable {
         handleProxyRequest(req, res, port, basePath);
        //});
      });
}

void GatewayServer::handleProxyRequest(const crow::request& req,
                                       crow::response&      res,
                                       int port,
                                       const std::string&   path) {
  RequestDTO request_info;
  request_info.method = crow::method_name(req.method);
  request_info.path = path;

  auto result = proxy_.forward(req, request_info, port);
  sendResponse(res, result.first, result.second);
}

void GatewayServer::handlePostRequest(const crow::request& req,
                                       crow::response&      res,
                                       int port,
                                       const std::string&   path) {
  RequestDTO request_info;
  request_info.method = crow::method_name(req.method);
  request_info.path = path;
  request_info.request_id = generateRequestID();

  cache_->set("request:" + request_info.request_id, "{ \"state\": \"queued\" }");

  std::thread([this, req, request_info, port] {
    proxy_.forward(req, request_info, port);  //TODO: rabit mq??
  }).detach();

  sendResponse(res, 202, request_info.request_id);
}

void GatewayServer::registerWebSocketRoutes() {
  std::string backend_url = fmt::format("ws://127.0.0.1:{}/ws",
      provider_->ports().notificationService);
  auto wsBridge = std::make_shared<WebSocketBridge>(backend_url);

  CROW_WEBSOCKET_ROUTE(app_, "/ws")
      .onopen([wsBridge, this](crow::websocket::connection& client) {
        wsBridge->onClientConnect(client);
        //metrics_->userConnected();
      })
      .onmessage([wsBridge, this](crow::websocket::connection& client, const std::string& data, bool) {
        wsBridge->onClientMessage(client, data);
        //metrics_->newMessage(client.get_remote_ip());
      })
      .onclose([wsBridge, this](crow::websocket::connection& client,
                          const std::string&           reason,
                          uint16_t code) {
        wsBridge->onClientClose(client, reason, code);
        //metrics_->userDisconnected();
      });
}

void GatewayServer::registerHealthCheck() {
  CROW_ROUTE(app_, "/healthz")([this](const crow::request&, crow::response& res) {
    json info = {{"status", "ok"},
                 {"timestamp", getCurrentTime()}};
    res.set_header("Content-Type", "application/json");
    sendResponse(res, provider_->statusCodes().success, info.dump());
  });
}
