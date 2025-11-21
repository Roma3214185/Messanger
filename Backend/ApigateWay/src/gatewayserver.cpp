#include "gatewayserver.h"

#include <chrono>
#include <cstdlib>
#include <iostream>

#include "Debug_profiling.h"
#include "ScopedRequestsTimer.h"
#include "websocketbridge.h"
#include "interfaces/ICacheService.h"

using json = nlohmann::json;
using namespace std::chrono_literals;
using std::string;

namespace {

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

constexpr int kInvalidTokenCode    = 401;
constexpr int kRateLimitExceedCode = 429;

constexpr int kMaxRequests = 300;
constexpr int kWindowSec = 900;

GatewayServer::GatewayServer(crow::SimpleApp& app, ICacheService* cache, IConfigProvider* provider)
    : app_(app)
    , provider_(provider)
    , cache_(cache)
    //, rate_limiter_(kMaxRequests, std::chrono::seconds(kWindowSec))
    //, proxy_repository_(proxy_repository)
    , metrics_(provider->ports().metrics)
    // , authProxy_(provider->ports().authService)
    // , chatProxy_(provider->ports().chatService)
    // , messageProxy_(provider->ports().messageService)
    // , notificationProxy_(provider->ports().notificationService)
    // , metrics_(metrics) {
  //   , exposer_(std::make_unique<prometheus::Exposer>("0.0.0.0:8089"))
  //   , registry_(std::make_shared<prometheus::Registry>())
  //   , request_counter_family_(prometheus::BuildCounter()
  //                                 .Name("api_gateway_requests_total")
  //                                 .Help("Total number of requests")
  //                                 .Register(*registry_))
  //   , request_latency_(prometheus::BuildHistogram()
  //                          .Name("api_gateway_request_duration_seconds")
  //                          .Help("Request duration in seconds")
  //                          .Register(*registry_)
  //                          .Add({},
  //                               prometheus::Histogram::BucketBoundaries{
  //                                   0.005, 0.01, 0.05, 0.1, 0.5, 1, 2, 5, 10})) {
  // exposer_->RegisterCollectable(registry_);
{
  registerRoutes();
}

void GatewayServer::run() {
  LOG_INFO("Starting API Gateway on port {}", provider_->ports().apigateService);
  app_.port(provider_->ports().apigateService).multithreaded().run();
}

string GatewayServer::extractToken(const crow::request& req) const {
  string authHeader = req.get_header_value("Authorization");
  if (authHeader.empty()) return {};
  return (authHeader.rfind("Bearer ", 0) == 0) ? authHeader.substr(7) : authHeader;
}

bool GatewayServer::checkRateLimit(const crow::request& req) {
  string ip = extractIP(req);
  return rate_limiter_.allow(ip);
}

void GatewayServer::sendResponde(crow::response& res, const RequestDTO& request_info, int res_code, const std::string& message, bool hitKey) {
  logger_.logResponse(res_code, message, request_info, hitKey);

  res.code = res_code;
  res.write(message);
  res.end();

  metrics_.requestEnded(request_info.path, res_code, hitKey);
}

void GatewayServer::registerRoutes() {
  registerRoute("/auth", proxy_repository_.auth(), false);
  registerRoute("/users", proxy_repository_.auth(), false);
  registerRoute("/chats", proxy_repository_.chat(), false);
  registerRoute("/messages", proxy_repository_.message(), false);
  registerRoute("/notification", proxy_repository_.notification(), false);
  registerHealthCheck();
  registerWebSocketRoutes();
}

void GatewayServer::registerRoute(const std::string& basePath,
                                  ProxyClient&       proxy,
                                  bool               requireAuth) {
  app_.route_dynamic(basePath + "/<path>")
      .methods("GET"_method,
               "POST"_method)([this, &proxy, basePath, requireAuth](
                                  const crow::request& req, crow::response& res, std::string path) {
        LOG_INFO("{}/{}", basePath, path);
        handleProxyRequest(req, res, proxy, basePath + "/" + path, requireAuth);
        LOG_INFO("Res code:{} and res: {}", res.code, res.body);
      });

  app_.route_dynamic(basePath).methods("GET"_method, "POST"_method)(
      [this, &proxy, basePath, requireAuth](const crow::request& req, crow::response& res) {
        LOG_INFO("{}", basePath);
        handleProxyRequest(req, res, proxy, basePath, requireAuth);
        LOG_INFO("Res code:{} and res: {}", res.code, res.body);
      });
}

std::string GatewayServer::extractIP(const crow::request& req) {
  auto ip = req.get_header_value("X-Forwarded-For");
  if (ip.empty()) {
    ip = req.remote_ip_address;
  }
  return ip;
}

bool GatewayServer::checkAuth(const crow::request& req, bool requireAuth) {
  if(!requireAuth) return true;
  std::string token     = extractToken(req);
  auto        userIdPtr = JwtUtils::verifyTokenAndGetUserId(token);
  if (!userIdPtr) return false;
  return true;
}

std::string makeCacheKey(const crow::request& req) {
  return "cache:" + getMethod(req.method) + ":" + req.url;
}

std::optional<nlohmann::json> GatewayServer::checkCache(std::string key) {
  return cache_->get(key);
}

void GatewayServer::saveInCache(const crow::request& req, std::string key, std::string value, std::chrono::milliseconds ttl) {
  if(req.method != crow::HTTPMethod::Get) return;
  cache_->set(key, value, ttl);
}

void GatewayServer::handleProxyRequest(const crow::request& req,
                                       crow::response&      res,
                                       ProxyClient&         proxy,
                                       const std::string&   path,
                                       bool                 requireAuth) {
  auto tracker = metrics_.getTracker(path);
  PROFILE_SCOPE(path.c_str());

  RequestDTO request_info;
  request_info.method = getMethod(req.method);
  request_info.path = path;
  logger_.logRequest(req, request_info);

  if(!checkRateLimit(req)) return sendResponde(res, request_info, kRateLimitExceedCode, "Rate limit exceeded");
  if(!checkAuth(req, requireAuth)) return sendResponde(res, request_info, kInvalidTokenCode, "Invalid token");

  auto key = makeCacheKey(req);
  if(auto cached = checkCache(key)) { // TODO: ttl good way to  check if data still valid??
    return sendResponde(res, request_info, provider_->statusCodes().success, cached.value(), true);
  }

  auto result = proxy.forward(req, path, request_info.method);

  saveInCache(req, key, result.second, std::chrono::milliseconds(60));
  sendResponde(res, request_info, result.first, result.second); // <- logs metrics about leave
}

void GatewayServer::registerWebSocketRoutes() {
  auto wsBridge = std::make_shared<WebSocketBridge>("ws://127.0.0.1:8086/ws");

  CROW_WEBSOCKET_ROUTE(app_, "/ws")
      .onopen([wsBridge, this](crow::websocket::connection& client) {
        wsBridge->onClientConnect(client);
        metrics_.userConnected();
      })
      .onmessage([wsBridge, this](crow::websocket::connection& client, const std::string& data, bool) {
        wsBridge->onClientMessage(client, data);
        metrics_.newMessage(client.get_remote_ip());
      })
      .onclose([wsBridge, this](crow::websocket::connection& client,
                          const std::string&           reason,
                          uint16_t code) {
        wsBridge->onClientClose(client, reason, code);
        metrics_.userDisconnected();
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
