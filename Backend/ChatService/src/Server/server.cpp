#include "server.h"
#include "Controller/controller.h"

Server::Server(const int port, DataBase& database)
    : port_(port)
    , db(database)
{
    controller = std::make_unique<Controller>(app, db);
    initRoutes();
}

void Server::initRoutes(){
    controller->handleRoutes();
}

void Server::run() {
    std::cout << "Starting Chat Server on port " << port_ << "\n";
    app.port(port_).multithreaded().run();
}
