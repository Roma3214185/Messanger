#ifndef SERVER_H
#define SERVER_H

#include "DataBase/database.h" // or class DataBase
#include <crow/crow.h>
#include "Controller/controller.h"

using ControllerPtr = std::unique_ptr<Controller>;

class Server
{
public:

    Server(const int port, DataBase &database);
    void run();

private:

    void initRoutes();

    int port_;
    DataBase db;
    crow::SimpleApp app;
    ControllerPtr controller;
};

#endif // SERVER_H
