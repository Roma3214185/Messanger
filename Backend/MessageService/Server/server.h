#ifndef SERVER_H
#define SERVER_H

#include <crow/crow.h>
#include "messagedatabase.h"
#include "controller.h"

using ControllerPtr = std::unique_ptr<Controller>;

class Server
{

public:

    Server(int port, DataBase& database);

    void run();

private:

    void handleRountes();

    crow::SimpleApp app;
    int port_;
    DataBase& db;
    ControllerPtr controller;
};

#endif // SERVER_H
