#include "gatewayserver.h"

#include <chrono>
#include <cstdlib>
#include <iostream>

#include "Debug_profiling.h"

using json = nlohmann::json;
using namespace std::chrono_literals;
using std::string;

static string getenv_or(const char* key, const char* def) {
  const char* v = std::getenv(key);
  return v ? string(v) : string(def);
}

GatewayServer::GatewayServer(const int& port)
    : port_(port),
      rateLimiter_(300, std::chrono::seconds(900)),
      authVerifier_(getenv_or("AUTH_SERVICE_URL", "http://localhost:8083")),
      authProxy_(getenv_or("AUTH_SERVICE_URL", "http://localhost:8083")),
      chatProxy_(getenv_or("PRODUCT_SERVICE_URL", "http://localhost:8081")),
      messageProxy_(getenv_or("ORDER_SERVICE_URL", "http://localhost:8082")),
      notificationProxy_(
          getenv_or("PAYMENT_SERVICE_URL", "http://localhost:8086")) {
  registerRoutes();
}

void GatewayServer::run() {
  std::cout << "Starting API Gateway on port " << port_ << "\n";
  app_.port(port_).multithreaded().run();
}

string GatewayServer::getMethod(const crow::HTTPMethod& method) const {
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
    res.code = 429;
    res.write("Rate limit exceeded");
    res.end();
    return false;
  }
  return true;
}

void GatewayServer::registerRoutes() {
  registrerHealthCheck();
  registerNotificationRoutes();
  registerUserRoutes();
  registerMessagesRoutes();
  registerChatRoutes();
  registerAuthRoutes();
  registerWebSocketRoutes();
}

void GatewayServer::registerAuthRoutes() {
  CROW_ROUTE(app_, "/auth/<path>")
      .methods(crow::HTTPMethod::GET, crow::HTTPMethod::POST)(
          [this](const crow::request& req, crow::response& res, string path) {
            LOG_INFO("chats/{}", path);
            string downstream_path = "/auth/" + path;
            auto result =
                authProxy_.forward(req, downstream_path, getMethod(req.method));
            res.code = result.first;
            res.write(result.second);
            res.end();
          });
}

void GatewayServer::registerChatRoutes() {
  CROW_ROUTE(app_, "/chats/<path>")
      .methods(crow::HTTPMethod::GET, crow::HTTPMethod::POST)(
          [this](const crow::request& req, crow::response& res, string path) {
            LOG_INFO("chats/{}", path);
            // if (!checkRateLimit(req, res)) return;

            // string token = extractToken(req);
            // auto v = authVerifier_.verify(token);
            // if (!v.first) {
            // LOG_ERROR("Invalid token");
            //   res.code = 401;
            //   res.write("Invalid token");
            //   res.end();
            //   return;
            // }

            string downstream_path = "/chats/" + path;
            auto result =
                chatProxy_.forward(req, downstream_path, getMethod(req.method));
            LOG_INFO("result code {} and res {}", result.first, result.second);
            res.code = result.first;
            res.write(result.second);
            res.end();
          });

  CROW_ROUTE(app_, "/chats")
      .methods(crow::HTTPMethod::GET, crow::HTTPMethod::POST)(
          [this](const crow::request& req, crow::response& res) {
            LOG_INFO("chats");
            // if (!checkRateLimit(req, res)) return;

            string token = extractToken(req);
            LOG_INFO("Token '{}'", token);
            auto userIdPtr = authVerifier_.verifyTokenAndGetUserId(token);
            if (!userIdPtr) {
              LOG_ERROR("Invalid token");
              res.code = 401;
              res.write("Invalid token");
              res.end();
              return;
            }

            string downstream_path = "/chats";
            auto result =
                chatProxy_.forward(req, downstream_path, getMethod(req.method));
            LOG_INFO("result code {} and res {}", result.first, result.second);
            res.code = result.first;
            res.write(result.second);
            res.end();
          });
}

void GatewayServer::registerMessagesRoutes() {
  CROW_ROUTE(app_, "/messages/<path>")
      .methods(crow::HTTPMethod::GET, crow::HTTPMethod::POST)(
          [this](const crow::request& req, crow::response& res, string path) {
            LOG_INFO("messages/{}", path);
            // if (!checkRateLimit(req, res)) return;

            // string token = extractToken(req);
            // auto v = authVerifier_.verify(token);
            // if (!v.first) {
            //   res.code = 401;
            //   res.write("Invalid token");
            //   res.end();
            //   return;
            // }

            string downstream_path = "/messages/" + path;
            auto result = messageProxy_.forward(req, downstream_path,
                                                getMethod(req.method));
            LOG_INFO("result code {} and res {}", result.first, result.second);
            res.code = result.first;
            res.write(result.second);
            res.end();
          });
}

void GatewayServer::registerNotificationRoutes() {
  CROW_ROUTE(app_, "/notification/<path>")
      .methods(crow::HTTPMethod::GET, crow::HTTPMethod::POST)(
          [this](const crow::request& req, crow::response& res, string path) {
            LOG_INFO("notification/{}", path);
            string downstream_path = "/notification/" + path;
            auto result = notificationProxy_.forward(req, downstream_path,
                                                     getMethod(req.method));
            LOG_INFO("result code {} and res {}", result.first, result.second);
            res.code = result.first;
            res.write(result.second);
            res.end();
          });
}

void GatewayServer::registerUserRoutes() {
  CROW_ROUTE(app_, "/users/<path>")
      .methods(crow::HTTPMethod::GET, crow::HTTPMethod::POST)(
          [this](const crow::request& req, crow::response& res, string path) {
            LOG_INFO("Users/{}", path);
            string downstream_path = "/users/" + path;
            auto result =
                authProxy_.forward(req, downstream_path, getMethod(req.method));
            LOG_INFO("result code {} and res {}", result.first, result.second);
            res.code = result.first;
            res.write(result.second);
            res.end();
          });
}

void GatewayServer::registerWebSocketRoutes() {
  CROW_WEBSOCKET_ROUTE(app_, "/ws")
  .onopen([&](crow::websocket::connection& client_ws) {
    spdlog::info("Frontend WebSocket connected: {}", client_ws.get_remote_ip());

    // Create backend IXWebSocket
    auto backend_ws = std::make_shared<ix::WebSocket>();
    backend_ws->setUrl("ws://127.0.0.1:8086/ws");

    crow::websocket::connection* client_ptr = &client_ws;

    // Backend -> frontend messages and events
    backend_ws->setOnMessageCallback([client_ptr](const ix::WebSocketMessagePtr& msg) {
      if (!client_ptr) return;

      switch (msg->type) {
      case ix::WebSocketMessageType::Open:
        spdlog::info("Backend WebSocket connected");
        break;
      case ix::WebSocketMessageType::Message:
        client_ptr->send_text(msg->str); // Forward message to frontend
        break;
      case ix::WebSocketMessageType::Close:
        spdlog::info("Backend WS closed: code={}, reason={}",
                     msg->closeInfo.code, msg->closeInfo.reason);
        break;
      case ix::WebSocketMessageType::Error:
        spdlog::error("Backend WebSocket error: {} ({})",
                      msg->errorInfo.reason, msg->errorInfo.http_status);
        break;
      default:
        break;
      }
    });

    backend_ws->start();
    client_to_backend[&client_ws] = backend_ws;
  })
      .onmessage([&](crow::websocket::connection& client_ws, const std::string& data, bool /*is_binary*/) {
        LOG_INFO("Send message: {}", data);

        auto it = client_to_backend.find(&client_ws);
        if (it != client_to_backend.end() && it->second) {
          it->second->send(data); // Forward frontend message to backend
        }
      })
      .onclose([&](crow::websocket::connection& client_ws, const std::string& reason, uint16_t code) {
        spdlog::info("Frontend WS closed: reason='{}', code={}", reason, code);

        auto it = client_to_backend.find(&client_ws);
        if (it != client_to_backend.end()) {
          auto backend_ws = it->second;
          if (backend_ws && backend_ws->getReadyState() == ix::ReadyState::Open) {
            backend_ws->close(); // Close backend safely
          }
          client_to_backend.erase(it);
        }
      });
}

void GatewayServer::registrerHealthCheck() {
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
