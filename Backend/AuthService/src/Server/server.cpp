#include "server.h"

Server::Server(const int& port, AuthManager* mgr)
    : port_(port)
    , manager(mgr)
{
    authController = std::make_unique<AuthController>(app, manager);
    initRoutes();
}

void Server::run() {
    std::cout << "Starting Auth Server on port " << port_ << "\n";
    app.port(port_).multithreaded().run();
}

void Server::initRoutes() {
    authController->initRoutes();
}
