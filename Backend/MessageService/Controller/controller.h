#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <crow.h>
#include <unordered_map>
#include <mutex>
#include <QtSql>
#include <QDebug>
#include "messagedatabase.h"

class Controller
{
    crow::SimpleApp& app_;
    DataBase& db;
public:
    Controller(crow::SimpleApp& app, DataBase& dataBase);
    void handleRoutes();
private:
    void handleGetMessagesFromChat();
    void handleSocket();
    void  userConnected(int userId, crow::websocket::connection* conn);
    void onSendMessage(int fromUser, int chatId, std::string text);

    std::unordered_map<int, crow::websocket::connection*> userSockets;
    std::mutex socketMutex;

};

#endif // CONTROLLER_H
