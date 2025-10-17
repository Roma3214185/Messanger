#include "server.h"
#include "controller.h"

Server::Server(int port, MessageManager& m)
    : port_(port)
    , manager(m)
{
    controller = std::make_unique<Controller>(app, manager);
    handleRountes();
}

void Server::handleRountes(){
    controller->handleRoutes();
}

void Server::run(){
    spdlog::info("[Message server is started on port '{}'",port_);
    app.port(port_).multithreaded().run();
}
