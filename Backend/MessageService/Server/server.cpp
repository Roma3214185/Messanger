#include "server.h"

#include "Controller/controller.h"
#include "RabbitMQClient/rabbitmqclient.h"

Server::Server(int port, MessageManager* message_manager,
               RabbitMQClient* mq_client)
    : port_(port) {
  LOG_INFO("Start: Controller created");
  controller_ = std::make_unique<Controller>(app_, mq_client, message_manager);
  LOG_INFO("Controller created");
  handleRoutes();
  LOG_INFO("Routes handled");
}

void Server::handleRoutes() { controller_->handleRoutes(); }

void Server::run() {
  spdlog::info("[Message server is started on port '{}'", port_);
  app_.port(port_).multithreaded().run();
}
