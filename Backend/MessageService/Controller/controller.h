#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <crow.h>
#include <unordered_map>
#include <mutex>
#include <QtSql>
#include <QDebug>
#include "messagedatabase.h"

using WebsocketPtr = crow::websocket::connection*;
using UserId = int;
using WebsocketByIdMap = std::unordered_map<UserId, WebsocketPtr>;

class Controller
{

public:

    Controller(crow::SimpleApp& app, DataBase& dataBase);
    void handleRoutes();

private:

    void handleGetMessagesFromChat();
    void handleSocket();
    void  userConnected(int userId, crow::websocket::connection* conn);
    void onSendMessage(int fromUser, int chatId, std::string text);

    WebsocketByIdMap userSockets;
    std::mutex socketMutex;
    crow::SimpleApp& app_;
    DataBase db;

};

#endif // CONTROLLER_H
