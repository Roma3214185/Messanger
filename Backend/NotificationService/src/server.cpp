#include "notificationservice/server.h"

#include <crow.h>

#include "notificationservice/managers/notificationmanager.h"
#include "notificationservice/CrowSocket.h"

Server::Server(int port, NotificationManager* notification_manager)
    : notification_manager_(notification_manager), notification_port_(port) {
  initRoutes();
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
        crow::json::wvalue json;
        json["type"] = "opened";
        LOG_INFO("Websocket is connected");
        conn.send_text(json.dump());
      })
      .onclose([&](crow::websocket::connection& conn, const std::string& reason, uint16_t code) {
        LOG_INFO("websocket disconnected, reason: '{}' and code '{}'", reason, code);
        CrowSocket socket(&conn);
        notification_manager_->deleteConnections(&socket);
      })
      .onmessage([&](crow::websocket::connection& conn, const std::string& data, bool is_binary) {
        CrowSocket socket(&conn);
        handleSocketOnMessage(&socket, data);
      });
}

void Server::handleSocketOnMessage(ISocket* socket, const std::string& data) {
  auto message_ptr = crow::json::load(data);
  if (!message_ptr) {
    LOG_ERROR("[onMessage] Failed in loading message");
    return;
  }

  if (!message_ptr.has("type")) {
    LOG_ERROR("[onMessage] No type");
    return;
  }

  const std::string& type = message_ptr["type"].s();
  if (type == "init") {
    if (!message_ptr.has("user_id")) {
      LOG_ERROR("[onMessage] No user_id in init message");
      return;
    }
    int user_id = message_ptr["user_id"].i();
    notification_manager_->userConnected(user_id, socket);
    LOG_INFO("[onMessage] Socket registered for userId '{}'", user_id);

  } else if (type == "send_message") {
    LOG_INFO("[temp] send_message");
    auto message = from_crow_json(message_ptr);
    notification_manager_->onSendMessage(message);

  } else if (type == "mark_read") {
    if (!message_ptr.has("readed_by")) {
      LOG_ERROR("[onMessage] No readed_by in mark_read message");
      return;
    }
    auto message = from_crow_json(message_ptr);
    int read_by = message_ptr["readed_by"].i();
    notification_manager_->onMarkReadMessage(message, read_by);

  } else {
    LOG_ERROR("[onMessage] Invalid type '{}'", type);
  }
}

