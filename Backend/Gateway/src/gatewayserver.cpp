#include "gatewayserver.h"

#include <chrono>
#include <cstdlib>
#include <fmt/format.h>
#include <uuid/uuid.h>
#include <nlohmann/json.hpp>

#include "Debug_profiling.h"
#include "websocketbridge.h"
#include "interfaces/ICacheService.h"
#include "middlewares/Middlewares.h"
#include "interfaces/IThreadPool.h"
#include "interfaces/IMetrics.h"
#include "interfaces/IRabitMQClient.h"

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
  return{ str };
}

std::string getContentType(const crow::request& req) {
  string content_type = req.get_header_value("content-type");
  if (content_type.empty()) content_type = "application/json";
  return content_type;
}


RequestDTO getRequestInfo(const crow::request& req, const std::string& path) {
  RequestDTO request_info;
  request_info.method = crow::method_name(req.method);
  request_info.path = path;
  request_info.body = req.body;
  request_info.content_type = getContentType(req);
  request_info.request_id = generateRequestID();

  auto keys = req.url_params.keys();
  for (const std::string& key : keys) {
    request_info.url_params.emplace(key, req.url_params.get(key));
  }

  for (auto const& header : req.headers) {
    request_info.headers.emplace_back(header.first, header.second);
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

void GatewayServer::registerRoute(const std::string& base_path,
                                  int port) {
  app_.route_dynamic(base_path + "/<path>")
      .methods("GET"_method)([this, port, base_path](
                                  const crow::request& req, crow::response& res, std::string path) {
        //pool_->enqueue([this, req = std::move(req), &res, port, base_path, path]() mutable {
          handleProxyRequest(req, res, port, base_path + "/" + path);
          //TODO: res.end() here or in handleProxyRequest
          //TODO: make async
       // });
      });

  app_.route_dynamic(base_path + "/<path>")
      .methods("POST"_method)([this, port, base_path](
                                  const crow::request& req, crow::response& res, std::string path) {
        //pool_->enqueue([this, req = std::move(req), &res, port, base_path, path]() mutable {
        handlePostRequest(req, res, port, base_path + "/" + path);
        //TODO: res.end() here or in handleProxyRequest
        //TODO: make async
        // });
      });

  app_.route_dynamic(base_path).methods("GET"_method)(
      [this, port, base_path](const crow::request& req, crow::response& res) {
        //pool_->enqueue([this, req = std::move(req), &res, port, base_path]() mutable {
         handleProxyRequest(req, res, port, base_path);
        //});
      });

  app_.route_dynamic(base_path).methods("POST"_method)(
      [this, port, base_path](const crow::request& req, crow::response& res) {
        //pool_->enqueue([this, req = std::move(req), &res, port, base_path]() mutable {
        handlePostRequest(req, res, port, base_path);
        //});
      });
}

void GatewayServer::handleProxyRequest(const crow::request& req,
                                       crow::response&      res,
                                       const int port,
                                       const std::string&   path) {
  RequestDTO request_info = getRequestInfo(req, path);

  auto result = proxy_.forward(request_info, port);
  sendResponse(res, result.first, result.second);
}

void GatewayServer::handlePostRequest(const crow::request& req, //todo: make handlers and unordered_map<request, handler>
                                       crow::response&      res,
                                       const int port,
                                       const std::string&   path) {
  RequestDTO request_info = getRequestInfo(req, path);
  cache_->set("request:" + request_info.request_id, "{ \"status\": \"queued\" }");
  auto json = nlohmann::json(request_info);
  json["port"] = port;

  const PublishRequest publish_request{ //todo: make PublishRequest and RequestDTO immutable
    .exchange = provider_->routes().exchange,
    .routing_key = provider_->routes().sendRequest,
    .message =  json.dump(),
    .exchange_type = "direct"
  };

  queue_->publish(publish_request);
  nlohmann::json responce;
  responce["status"] = "queued";
  responce["request_id"] = request_info.request_id;
  sendResponse(res, provider_->statusCodes().accepted, responce.dump());
}

void GatewayServer::subscribeOnNewRequest() {
  SubscribeRequest subscribe_request{
    .queue = provider_->routes().sendRequest,
    .exchange = provider_->routes().exchange,
    .routing_key = provider_->routes().sendRequest,
    .exchange_type = "direct"
  };

  queue_->subscribe(subscribe_request,
                    [this](const std::string& event, const std::string& payload) {
                      LOG_INFO("I in subscribe with event {} and payload {}", event, payload);
    auto request_info_port = [payload]() -> std::optional<std::pair<RequestDTO, int>> {
                        try {
                          auto json = nlohmann::json(payload);
                          RequestDTO dto = nlohmann::json::parse(payload);
                          const int port = json["port"];

                          return std::make_pair(dto, port);
                        } catch (const std::exception& e) {
                          LOG_ERROR("Can't parse RequestDTO from payload {}: {}", payload, e.what());
                          return std::nullopt;
                        } catch (...) {
                          LOG_ERROR("Unknown error while parsing RequestDTO from payload {}", payload);
                          return std::nullopt;
                        }
                      }();

    if(!request_info_port) return;
    auto [request_info, port] = *request_info_port;
    auto result = proxy_.forward(request_info, port);
    LOG_INFO("Finished result in queue_->subscribe, request_info->request_id = {}, status_code = {}, body = {}",
      request_info.request_id, std::to_string(result.first), result.second.substr(0, result.second.length()));

    cache_->set("request:" + request_info.request_id, "{\"status\":\"finished\"}");
    cache_->set("request_id:" + request_info.request_id, std::to_string(result.first));
    cache_->set("request_body:" + request_info.request_id,
                result.second.substr(0, result.second.length() - 1) + ",\"status\":\"finished\"}"); //todo: fully refactor server responce JsonObject,
                                                                                                  // return ["error"], ["body"], maybe ["code"]
  });
}

void GatewayServer::registerRequestRoute() {
  CROW_ROUTE(app_, "/request/<string>/status")
  .methods("GET"_method)
      ([this](const crow::request& /*req*/, crow::response& res, std::string task_id) {
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
  auto ws_bridge = std::make_shared<WebSocketBridge>(backend_url);

  CROW_WEBSOCKET_ROUTE(app_, "/ws")
      .onopen([ws_bridge, this](crow::websocket::connection& client) {
        ws_bridge->onClientConnect(client);
        //metrics_->userConnected();
      })
      .onmessage([ws_bridge, this](crow::websocket::connection& client, const std::string& data, bool) {
        ws_bridge->onClientMessage(client, data);
        //metrics_->newMessage(client.get_remote_ip());
      })
      .onclose([ws_bridge, this](crow::websocket::connection& client,
                          const std::string&           reason,
                          uint16_t code) {
        ws_bridge->onClientClose(client, reason, code);
        //metrics_->userDisconnected();
      });
}

void GatewayServer::registerHealthCheck() {
  CROW_ROUTE(app_, "/healthz")([this](const crow::request&, crow::response& res) {
    nlohmann::json info = {{"status", "ok"},
                 {"timestamp", getCurrentTime()}};
    res.set_header("Content-Type", "application/json");
    sendResponse(res, provider_->statusCodes().success, info.dump());
  });
}
