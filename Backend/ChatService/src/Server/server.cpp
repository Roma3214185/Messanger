#include "server.h"

#include "Controller/controller.h"
#include "Debug_profiling.h"

Server::Server(const int port, DataBase& database) : port_(port), database_(database) {
  controller_ = std::make_unique<Controller>(app_, database_);
  initRoutes();
}

void Server::initRoutes() { controller_->handleRoutes(); }

void Server::run() {
  LOG_INFO("Chat microservice is running on port {}", port_);
  app_.port(port_).multithreaded().run();
}
