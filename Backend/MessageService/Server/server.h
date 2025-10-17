#ifndef SERVER_H
#define SERVER_H

#include <crow/crow.h>
#include "controller.h"
#include "../MessageManager/MessageManager.h"

using ControllerPtr = std::unique_ptr<Controller>;

class Server
{

public:

    Server(int port, MessageManager& manager);

    void run();

private:

    void handleRountes();

    crow::SimpleApp app;
    int port_;
    MessageManager& manager;
    ControllerPtr controller;
};

#endif // SERVER_H
