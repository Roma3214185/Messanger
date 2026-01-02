#include "notificationservice/server.h"

#include <crow.h>

#include "notificationservice/managers/notificationmanager.h"
#include "notificationservice/CrowSocket.h"
#include "handlers/MessageHanldlers.h"

Server::Server(int port, NotificationManager* notification_manager)
    : notification_manager_(notification_manager), notification_port_(port) {
  initRoutes();
  initHanlers();
}

void Server::initHanlers() {
  handlers_["init"] = std::make_unique<InitMessageHandler>();
  handlers_["send_message"] = std::make_unique<SendMessageHandler>();
  handlers_["read_message"] = std::make_unique<MarkReadMessageHandler>();
}

void Server::run() {
  LOG_INFO("Notication service is running on '{}'", notification_port_);
  app_.port(notification_port_).multithreaded().run();
}

void Server::initRoutes() { handleSocketRoutes(); }

void Server::handleSocketRoutes() {
  CROW_ROUTE(app_, "/ws")
      .websocket(&app_)
      .onopen([&](crow::websocket::connection& conn) {
        auto socket = std::make_shared<CrowSocket>(&conn);
        {
          std::lock_guard<std::mutex> lock(ws_mutex);
          active_sockets.insert(socket);
        }

        crow::json::wvalue json;
        json["type"] = "opened";
        LOG_INFO("Websocket is connected");
        conn.send_text(json.dump());
      })
      .onclose([&](crow::websocket::connection& conn, const std::string& reason, uint16_t code) {
        auto socket = findSocket(&conn);
        if (!socket) {
          LOG_WARN("Socket not found for onclose");
          return;
        }

        notification_manager_->deleteConnections(socket);

        std::lock_guard<std::mutex> lock(ws_mutex);
        active_sockets.erase(socket);
        LOG_INFO("WebSocket disconnected: '{}' code {}", reason, code);


      })
      .onmessage([&](crow::websocket::connection& conn, const std::string& data, bool /*is_binary*/) {
        auto socket = findSocket(&conn); // use the existing wrapper
        if (!socket) {
          LOG_ERROR("Socket not found for onmessage");
          return;
        }

        handleSocketOnMessage(socket, data);
      });
}

void Server::handleSocketOnMessage(const std::shared_ptr<ISocket>& socket, const std::string& data) {
  auto message_ptr = crow::json::load(data);
  if (!message_ptr) {
    LOG_ERROR("[onMessage] Failed in loading data: {}", data);
    return;
  }

  if (!message_ptr.has("type")) {
    LOG_ERROR("[onMessage] No type");
    return;
  }

  const std::string& type = message_ptr["type"].s();
  if (auto it = handlers_.find(type); it != handlers_.end()) {
    LOG_INFO("Type is valid {}", type);
    it->second->handle(message_ptr, std::move(socket), *notification_manager_);
  } else {
    LOG_ERROR("Type isn't valid {}", type);
  }
}

SocketPtr Server::findSocket(crow::websocket::connection* conn) {
  {
    std::lock_guard<std::mutex> lock(ws_mutex);
    for (auto& socket : active_sockets) {
      if (auto crowSocket = std::dynamic_pointer_cast<CrowSocket>(socket)) {
        if (crowSocket->isSameAs(conn)) {
          return socket;
        }
      }
    }
  }

  return nullptr; // not found
}

