#include "gatewayserver.h"

#include <chrono>
#include <cstdlib>
#include <iostream>

#include "Debug_profiling.h"
#include "ScopedRequestsTimer.h"
#include "WebSocketBridge/websocketbridge.h"

using json = nlohmann::json;
using namespace std::chrono_literals;
using std::string;

namespace {

static string getenv_or(const char* key, const char* def) {
  const char* v = std::getenv(key);
  return v ? string(v) : string(def);
}

std::string getMethod(const crow::HTTPMethod& method) {
  switch (method) {
    case crow::HTTPMethod::GET:
      return "GET";
    case crow::HTTPMethod::Delete:
      return "DELETE";
    case crow::HTTPMethod::Put:
      return "PUT";
    default:
      return "POST";
  }
}

}  // namespace

constexpr int kInvalidTokenCode = 401;
constexpr int kRateLimitExceedCode = 429;

GatewayServer::GatewayServer(int port)
    : port_(port),
      rateLimiter_(300, std::chrono::seconds(900)),
      authProxy_(getenv_or("AUTH_SERVICE_URL", "http://localhost:8083")),
      chatProxy_(getenv_or("PRODUCT_SERVICE_URL", "http://localhost:8081")),
      messageProxy_(getenv_or("ORDER_SERVICE_URL", "http://localhost:8082")),
      notificationProxy_(
          getenv_or("PAYMENT_SERVICE_URL", "http://localhost:8086")),
      exposer_(std::make_unique<prometheus::Exposer>("0.0.0.0:8089")),
      registry_(std::make_shared<prometheus::Registry>()),
      request_counter_family_(prometheus::BuildCounter()
                                  .Name("api_gateway_requests_total")
                                  .Help("Total number of requests")
                                  .Register(*registry_)),
      request_latency_(
          prometheus::BuildHistogram()
              .Name("api_gateway_request_duration_seconds")
              .Help("Request duration in seconds")
              .Register(*registry_)
              .Add({}, prometheus::Histogram::BucketBoundaries{
                           0.005, 0.01, 0.05, 0.1, 0.5, 1, 2, 5, 10})) {
  exposer_->RegisterCollectable(registry_);
  registerRoutes();
}

void GatewayServer::run() {
  std::cout << "Starting API Gateway on port " << port_ << "\n";
  app_.port(port_).multithreaded().run();
}

string GatewayServer::extractToken(const crow::request& req) const {
  string authHeader = req.get_header_value("Authorization");
  if (authHeader.empty()) return {};
  return (authHeader.rfind("Bearer ", 0) == 0) ? authHeader.substr(7)
                                               : authHeader;
}

bool GatewayServer::checkRateLimit(const crow::request& req,
                                   crow::response& res) {
  string ip = req.remote_ip_address;
  if (!rateLimiter_.allow(ip)) {
    res.code = kRateLimitExceedCode;
    res.write("Rate limit exceeded");
    res.end();
    return false;
  }
  return true;
}

void GatewayServer::registerRoutes() {
  registerRoute("/auth", authProxy_, false);
  registerRoute("/users", authProxy_, false);
  registerRoute("/chats", chatProxy_, false);
  registerRoute("/messages", messageProxy_, false);
  registerRoute("/notification", notificationProxy_, false);
  registerHealthCheck();
  registerWebSocketRoutes();
}

void GatewayServer::registerRoute(const std::string& basePath,
                                  ProxyClient& proxy,
                                  bool requireAuth) {
  app_.route_dynamic(basePath + "/<path>")
  .methods("GET"_method, "POST"_method)
      ([this, &proxy, basePath, requireAuth](const crow::request& req, crow::response& res, std::string path) {
        handleProxyRequest(req, res, proxy, basePath + "/" + path, requireAuth);
      });

  app_.route_dynamic(basePath)
      .methods("GET"_method, "POST"_method)
      ([this, &proxy, basePath, requireAuth](const crow::request& req, crow::response& res) {
        handleProxyRequest(req, res, proxy, basePath, requireAuth);
      });
}

void GatewayServer::handleProxyRequest(
    const crow::request& req,
    crow::response& res,
    ProxyClient& proxy,
    const std::string& path,
    bool requireAuth) {
  std::string method = getMethod(req.method);
  ScopedRequestMetrics metrics(request_counter_family_, request_latency_, path, method);
  PROFILE_SCOPE(path.c_str());

  if (requireAuth) {
    std::string token = extractToken(req);
    auto userIdPtr = JwtUtils::verifyTokenAndGetUserId(token);
    if (!userIdPtr) {
      LOG_ERROR("Invalid token for path {}", path);
      res.code = kInvalidTokenCode;
      res.write("Invalid token");
      res.end();
      return;
    }
  }

  auto result = proxy.forward(req, path, method);
  metrics.setStatus(result.first);
  res.code = result.first;
  res.write(result.second);
  res.end();
}

void GatewayServer::registerWebSocketRoutes() {
  auto wsBridge = std::make_shared<WebSocketBridge>("ws://127.0.0.1:8086/ws");

  CROW_WEBSOCKET_ROUTE(app_, "/ws")
      .onopen([wsBridge](crow::websocket::connection& client) {
        wsBridge->onClientConnect(client);
      })
      .onmessage([wsBridge](crow::websocket::connection& client, const std::string& data, bool) {
        wsBridge->onClientMessage(client, data);
      })
      .onclose([wsBridge](crow::websocket::connection& client, const std::string& reason, uint16_t code) {
        wsBridge->onClientClose(client, reason, code);
      });
}

void GatewayServer::registerHealthCheck() {
  CROW_ROUTE(app_, "/healthz")([](const crow::request&, crow::response& res) {
    json info = {
        {"status", "ok"},
        {"timestamp", std::chrono::duration_cast<std::chrono::milliseconds>(
                          std::chrono::system_clock::now().time_since_epoch())
                          .count()}};
    res.set_header("Content-Type", "application/json");
    res.write(info.dump());
    res.end();
  });
}
