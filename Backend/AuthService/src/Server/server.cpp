#include "server.h"
#include "../../../DebugProfiling/Debug_profiling.h"

Server::Server(const int& port, AuthManager* mgr)
    : port_(port)
    , manager(mgr)
{
    authController = std::make_unique<AuthController>(app, manager);
    initRoutes();
}

void Server::run() {
    LOG_INFO( "Starting Auth Server on port '{}'", port_);
    app.port(port_).multithreaded().run();
}

void Server::initRoutes() {
    authController->initRoutes();
}
