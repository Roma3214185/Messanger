#include "server.h"
#include "controller.h"

Server::Server(int port, DataBase& database)
    : port_(port)
    , db(database)
{
    controller = std::make_unique<Controller>(app, db);
    handleRountes();
}

void Server::handleRountes(){
    controller->handleRoutes();
}

void Server::run(){
    qDebug() << "[INFO] Message server is started on port: " << port_;
    app.port(port_).multithreaded().run();
}
