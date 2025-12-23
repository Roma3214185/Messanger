#include "gatewayserver.h"

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <fmt/format.h>
#include <uuid/uuid.h>
#include <nlohmann/json.hpp>

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
#include "interfaces/IRabitMQClient.h"

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

std::string getContentType(const crow::request& req) {
  string content_type = req.get_header_value("content-type");
  if (content_type.empty()) content_type = "application/json";
  return content_type;
}


RequestDTO getRequestInfo(const crow::request& req, const std::string& path, int port) {
  RequestDTO request_info;
  request_info.port = port;
  request_info.method = crow::method_name(req.method);
  request_info.path = path;
  request_info.body = req.body;
  request_info.content_type = getContentType(req);
  request_info.request_id = generateRequestID();

  auto keys = req.url_params.keys();
  for (const std::string& key : keys) {
    request_info.url_params.emplace(key, req.url_params.get(key));
  }

  for (auto const& h : req.headers) {
    request_info.headers.emplace_back(h.first, h.second);
  }

  return request_info;
}

}  // namespace

GatewayServer::GatewayServer(GatewayApp& app, IClient* client, ICacheService* cache, IThreadPool* pool,
                             IConfigProvider* provider, IRabitMQClient* queue)
    : app_(app)
    , provider_(provider)
    , cache_(cache)
    , proxy_(client)
    , pool_(pool)
    , queue_(queue) {

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
  subscribeOnNewRequest();
}

void GatewayServer::registerRoute(const std::string& basePath,
                                  int port) {
  app_.route_dynamic(basePath + "/<path>")
      .methods("GET"_method)([this, port, basePath](
                                  const crow::request& req, crow::response& res, std::string path) {
        //pool_->enqueue([this, req = std::move(req), &res, port, basePath, path]() mutable {
          handleProxyRequest(req, res, port, basePath + "/" + path);
          //TODO: res.end() here or in handleProxyRequest
          //TODO: make async
       // });
      });

  app_.route_dynamic(basePath + "/<path>")
      .methods("POST"_method)([this, port, basePath](
                                  const crow::request& req, crow::response& res, std::string path) {
        //pool_->enqueue([this, req = std::move(req), &res, port, basePath, path]() mutable {
        handlePostRequest(req, res, port, basePath + "/" + path);
        //TODO: res.end() here or in handleProxyRequest
        //TODO: make async
        // });
      });

  app_.route_dynamic(basePath).methods("GET"_method)(
      [this, port, basePath](const crow::request& req, crow::response& res) {
        //pool_->enqueue([this, req = std::move(req), &res, port, basePath]() mutable {
         handleProxyRequest(req, res, port, basePath);
        //});
      });

  app_.route_dynamic(basePath).methods("POST"_method)(
      [this, port, basePath](const crow::request& req, crow::response& res) {
        //pool_->enqueue([this, req = std::move(req), &res, port, basePath]() mutable {
        handlePostRequest(req, res, port, basePath);
        //});
      });
}

void GatewayServer::handleProxyRequest(const crow::request& req,
                                       crow::response&      res,
                                       int port,
                                       const std::string&   path) {
  RequestDTO request_info = getRequestInfo(req, path, port);

  auto result = proxy_.forward(request_info);
  sendResponse(res, result.first, result.second);
}

void GatewayServer::handlePostRequest(const crow::request& req,
                                       crow::response&      res,
                                       int port,
                                       const std::string&   path) {
  RequestDTO request_info = getRequestInfo(req, path, port);
  cache_->set("request:" + request_info.request_id, "{ \"status\": \"queued\" }");

  PublishRequest publish_request{
    .exchange = provider_->routes().exchange,
    .routingKey = provider_->routes().sendRequest,
    .message =  nlohmann::json(request_info).dump(),
    .exchangeType = "direct"
  };

  queue_->publish(publish_request);
  nlohmann::json responce;
  responce["status"] = "queued";
  responce["request_id"] = request_info.request_id;
  sendResponse(res, 202, responce.dump());
}

void GatewayServer::subscribeOnNewRequest() {
  SubscribeRequest subscribe_request{
    .queue = provider_->routes().sendRequest,
    .exchange = provider_->routes().exchange,
    .routingKey = provider_->routes().sendRequest,
    .exchangeType = "direct"
  };

  queue_->subscribe(subscribe_request,
                    [this](const std::string& event, const std::string& payload) {
                      LOG_INFO("I in subscribe with event {} and payload {}", event, payload);
    auto request_info = [payload]() -> std::optional<RequestDTO> {
                        try {
                          return std::make_optional(nlohmann::json::parse(payload));
                        } catch (const std::exception& e) {
                          LOG_ERROR("Can't parse RequestDTO from payload {}: {}", payload, e.what());
                          return std::nullopt;
                        } catch (...) {
                          LOG_ERROR("Unknown error while parsing RequestDTO from payload {}", payload);
                          return std::nullopt;
                        }
                      }();

    if(!request_info) return;
    auto result = proxy_.forward(*request_info);
    LOG_INFO("Finished result in queue_->subscribe, request_info->request_id = {}, status_code = {}, body = {}",
      request_info->request_id, std::to_string(result.first), result.second.substr(0, result.second.length()));

    cache_->set("request:" + request_info->request_id, "{\"status\":\"finished\"}");
    cache_->set("request_id:" + request_info->request_id, std::to_string(result.first));
    cache_->set("request_body:" + request_info->request_id,
                result.second.substr(0, result.second.length() - 1) + ",\"status\":\"finished\"}");
  });

}

void GatewayServer::registerRequestRoute() {
  CROW_ROUTE(app_, "/request/<string>/status")
  .methods("GET"_method)
      ([this](const crow::request& req, crow::response& res, std::string task_id) {
        LOG_INFO("Request id = {}", task_id);
        std::optional<std::string> status = cache_->get("request:" + task_id);
        nlohmann::json responce;
        std::optional<std::string> id = cache_->get("request_id:" + task_id);
        std::optional<std::string> body = cache_->get("request_body:" + task_id);

        if (!status || !id || !body) {
          responce["status"] = "not_found";
          if(!status) LOG_INFO("Not found status");
          if(!id) LOG_INFO("Not found id");
          if(!body) LOG_INFO("Not found body");
          sendResponse(res, 404, responce.dump());
        } else {
          LOG_INFO("Status found, return {}", *body);
          sendResponse(res, stoi(*id), *body);
        }
      });
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
