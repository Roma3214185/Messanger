#include "server.h"

#include "Controller/controller.h"
#include "Debug_profiling.h"

Server::Server(const int port, ChatManager* manager) : port_(port), manager_(manager) {
  controller_ = std::make_unique<Controller>(app_, manager_);
  initRoutes();
}

void Server::initRoutes() { controller_->handleRoutes(); }

void Server::run() {
  LOG_INFO("Chat microservice is running on port {}", port_);
  app_.port(port_).multithreaded().run();
}
