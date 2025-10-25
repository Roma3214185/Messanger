#include "server.h"
#include "controller.h"
#include "../../RabbitMQClient/rabbitmqclient.h"

Server::Server(int port, MessageManager& m, RabbitMQClient& mq)
    : port_(port)
{
    controller = std::make_unique<Controller>(app, mq, m);
    handleRountes();
}

void Server::handleRountes(){
    controller->handleRoutes();
}

void Server::run(){
    spdlog::info("[Message server is started on port '{}'",port_);
    app.port(port_).multithreaded().run();
}
