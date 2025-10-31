#include "server.h"

#include <crow/crow.h>

#include "notificationmanager.h"

Server::Server(int port, NotificationManager& notification_manager)
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
        LOG_INFO("Websocket is connected");
      })
      .onclose([&](crow::websocket::connection& conn, const std::string& reason,
                   uint16_t code) {
        LOG_INFO("websocket disconnected, reason: '{}' and code '{}'", reason,
                 code);
        notification_manager_.deleteConnections(&conn);
      })
      .onmessage([&](crow::websocket::connection& conn, const std::string& data,
                     bool is_binary) {
        handleSocketOnMessage(conn, data, is_binary);
      });
}

void Server::handleSocketOnMessage(crow::websocket::connection& conn,
                                   const std::string& data, bool is_binary) {
  auto message_ptr = crow::json::load(data);
  LOG_INFO("HANDLE SOCKET ON MESSAGE");
  if (!message_ptr) {
    LOG_ERROR("[onMessage] Failed in loading message");
    return;
  }

  if (!message_ptr.has("type")) {
    LOG_ERROR("[onMessage] No type");
    return;
  }

  if (message_ptr["type"].s() == "init") {
    int user_id = message_ptr["userId"].i();
    notification_manager_.userConnected(user_id, &conn);
    LOG_INFO("[onMessage] Socket is registered for userId '{}'", user_id);
  } else if (message_ptr["type"].s() == "send_message") {
    auto message = from_crow_json(message_ptr);
    notification_manager_.onSendMessage(message);
  } else if (message_ptr["type"].s() == "mark_read") {
    auto message = from_crow_json(message_ptr);
    int read_by = message_ptr["receiver_id"].i();
    notification_manager_.onMarkReadMessage(message, read_by);
  } else {
    LOG_ERROR("[onMessage] Invalid type");
  }
}
