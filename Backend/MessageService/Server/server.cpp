#include "server.h"

#include "Controller/controller.h"
#include "RabbitMQClient/rabbitmqclient.h"

Server::Server(int port, MessageManager* message_manager,
               RabbitMQClient* mq_client)
    : port_(port) {
  controller_ = std::make_unique<Controller>(app_, mq_client, message_manager);
  handleRoutes();
}

void Server::handleRoutes() {
  handleGetMessagesFromChat();
}

void Server::handleGetMessagesFromChat() {
  CROW_ROUTE(app_, "/messages/<int>")
  .methods(crow::HTTPMethod::GET)([&](const crow::request& req, crow::response& res, int chat_id) {
    PROFILE_SCOPE("/message/<int>");
    controller_->getMessagesFromChat(req, chat_id, res);
    LOG_INFO("Response code: {} | Body: {}", res.code, res.body);
    });
}


void Server::run() {
  spdlog::info("[Message server is started on port '{}'", port_);
  app_.port(port_).multithreaded().run();
}
