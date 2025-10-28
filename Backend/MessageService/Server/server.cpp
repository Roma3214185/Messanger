#include "server.h"

#include "Controller/controller.h"
#include "RabbitMQClient/rabbitmqclient.h"

Server::Server(int port, MessageManager* message_manager,
               RabbitMQClient* mq_client)
    : port_(port) {
  controller_ = std::make_unique<Controller>(app_, mq_client, message_manager);
  handleRoutes();
}

void Server::handleRoutes() { controller_->handleRoutes(); }

void Server::run() {
  spdlog::info("[Message server is started on port '{}'", port_);
  app_.port(port_).multithreaded().run();
}
