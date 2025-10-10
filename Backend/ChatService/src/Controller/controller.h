#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <crow/crow.h>
#include "database.h"


class Controller
{
    crow::SimpleApp& app_;
    DataBase db;
public:
    Controller(crow::SimpleApp& app, DataBase& dataBase);
    void handleRoutes();
private:
    void handleCreatingPrivateChat();
    void handleGetAllChats();
    void handleGetAllChatsById();
    void handleGetAllChatsMembers();
};

#endif // CONTROLLER_H
