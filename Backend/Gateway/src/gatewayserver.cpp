#include "gatewayserver.h"

#include <fmt/format.h>
#include <uuid/uuid.h>
#include <ixwebsocket/IXWebSocket.h>

#include <chrono>
#include <cstdlib>
#include <nlohmann/json.hpp>

#include "Debug_profiling.h"
#include "config/Routes.h"
#include "config/ports.h"
#include "interfaces/ICacheService.h"
#include "interfaces/IMetrics.h"
#include "interfaces/IRabitMQClient.h"
#include "interfaces/IThreadPool.h"
#include "middlewares/Middlewares.h"
#include "websocketbridge.h"
#include "GatewayController.h"
#include "utils.h"

GatewayServer::GatewayServer(GatewayApp &app, GatewayController* controller)
    : app_(app), controller_(controller) {}

void GatewayServer::run() {
    LOG_INFO("Starting API Gateway on port {}", Config::Ports::apigateService);
    app_.port(Config::Ports::apigateService).multithreaded().run();
}

void GatewayServer::registerRoutes() {
    registerRoute("/auth", Config::Ports::authService);
    registerRoute("/users", Config::Ports::authService);
    registerRoute("/chats", Config::Ports::chatService);
    registerRoute("/messages", Config::Ports::messageService);
    registerRoute("/notification", Config::Ports::notificationService);
    registerRequestRoute();
    registerHealthCheck();
    registerWebSocketRoutes();
}

void GatewayServer::registerRoute(const std::string &base_path, int port) {
    app_.route_dynamic(base_path + "/<path>")
    .methods("GET"_method, crow::HTTPMethod::DELETE, crow::HTTPMethod::PUT)(
        [this, port, base_path](const crow::request &req, crow::response &res,
                                // cppcheck-suppress passedByValue
                                std::string path /*NOLINT(performance-unnecessary-value-param)*/) {
            controller_->handleProxyRequest(req, res, port, base_path + "/" + path);
        });

    app_.route_dynamic(base_path + "/<path>")
        .methods("POST"_method)(
            [this, port, base_path](const crow::request &req, crow::response &res,
                                    // cppcheck-suppress passedByValue
                                    std::string path /*NOLINT(performance-unnecessary-value-param)*/) {
                controller_->handlePostRequest(req, res, port, base_path + "/" + path);
            });

    app_.route_dynamic(base_path).methods("GET"_method, crow::HTTPMethod::DELETE, crow::HTTPMethod::PUT)(
        [this, port, base_path](const crow::request &req, crow::response &res) {
            controller_->handleProxyRequest(req, res, port, base_path);
        });

    app_.route_dynamic(base_path).methods("POST"_method)(
        [this, port, base_path](const crow::request &req, crow::response &res) {
            controller_->handlePostRequest(req, res, port, base_path);
        });
}


void GatewayServer::registerRequestRoute() {
    CROW_ROUTE(app_, "/request/<string>/status")
    .methods("GET"_method)([this](const crow::request & /*req*/, crow::response &res, std::string task_id) {
        controller_->handleRequestRoute(res, task_id);
    });
}

void GatewayServer::registerWebSocketRoutes() {
    std::string backend_url = fmt::format("ws://127.0.0.1:{}/ws", Config::Ports::notificationService);
    auto ws_bridge = std::make_shared<WebSocketBridge>(backend_url);

    CROW_WEBSOCKET_ROUTE(app_, "/ws")
        .onopen([ws_bridge](crow::websocket::connection &client) {
            ws_bridge->onClientConnect(client);
            // metrics_->userConnected();
        })
        .onmessage([ws_bridge](crow::websocket::connection &client, const std::string &data, bool) {
            ws_bridge->onClientMessage(client, data);
            // metrics_->newMessage(client.get_remote_ip());
        })
        .onclose([ws_bridge](crow::websocket::connection &client, const std::string &reason, uint16_t code) {
            ws_bridge->onClientClose(client, reason, code);
            // metrics_->userDisconnected();
        });
}

void GatewayServer::registerHealthCheck() {
    CROW_ROUTE(app_, "/healthz")
    ([this](const crow::request&) {
        nlohmann::json info = {{"status", "ok"}, {"timestamp", utils::getCurrentTime()}};
        return crow::response(Config::StatusCodes::success, info.dump());
    });
}
