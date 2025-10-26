#ifndef SERVER_H
#define SERVER_H

#include <crow.h>

#include "controller.h"
#include "MessageManager.h"

using ControllerPtr = std::unique_ptr<Controller>;

class Server {
public:
    Server(int port, MessageManager& manager, RabbitMQClient& mq);
    void run();

private:
    void handleRoutes();
    crow::SimpleApp app;
    int port_;
    ControllerPtr controller;
};

#endif // SERVER_H
