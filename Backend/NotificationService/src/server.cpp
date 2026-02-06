#include "notificationservice/server.h"

#include <crow.h>

#include "SocketHandlersRepositoty.h"
#include "handlers/MessageHanldlers.h"
#include "notificationservice/CrowSocket.h"
#include "notificationservice/SocketRepository.h"
#include "notificationservice/managers/NotificationOrchestrator.h"
#include "notificationservice/ISubscriber.h"

Server::Server(int port, IActiveSocketRepository *socket_repository,
               SocketHandlersRepository *socket_handlers_repository, ISubscriber* subscriber)
    : notification_port_(port),
      socket_handlers_repository_(socket_handlers_repository),
      active_sockets_(socket_repository),
      subscriber_(subscriber) {
}

void Server::run() {
  initRoutes();
  subscriber_->subscribeAll();
  LOG_INFO("Notication service is running on '{}'", notification_port_);
  app_.port(notification_port_).multithreaded().run();
}

void Server::initRoutes() { handleSocketRoutes(); }

void Server::handleSocketRoutes() {
  CROW_ROUTE(app_, "/ws")
      .websocket(&app_)
      .onopen([&](crow::websocket::connection &conn) {
        auto socket = std::make_shared<CrowSocket>(&conn);
        active_sockets_->addConnection(socket);
        LOG_INFO("Websocket is connected");
        conn.send_text(nlohmann::json{{"type", "opened"}}.dump());
      })
      .onclose([&](crow::websocket::connection &conn, const std::string &reason, uint16_t code) {
        auto socket = active_sockets_->findSocket(&conn);
        if (!socket) {
          LOG_WARN("Socket not found for onclose");
          return;
        }

        active_sockets_->deleteConnection(socket);
        LOG_INFO("WebSocket disconnected: '{}' code {}", reason, code);
      })
      .onmessage([&](crow::websocket::connection &conn, const std::string &data, bool /*is_binary*/) {
        auto socket = active_sockets_->findSocket(&conn);  // use the existing wrapper
        if (!socket) {
          LOG_ERROR("Socket not found for onmessage");
          return;
        }

        handleSocketOnMessage(socket, data);
      });
}

void Server::handleSocketOnMessage(const std::shared_ptr<ISocket> &socket, const std::string &data) {
  LOG_INFO("Data from socket {}", data);
  auto message_ptr = crow::json::load(data);
  if (!message_ptr) {
    LOG_ERROR("[onMessage] Failed in loading data: {}", data);
    return;
  }

  socket_handlers_repository_->handle(message_ptr, socket);
}
