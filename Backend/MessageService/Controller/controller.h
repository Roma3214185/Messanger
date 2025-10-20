#ifndef CONTROLLER_H
#define CONTROLLER_H

#ifdef signals
#undef signals
#endif
#ifdef slots
#undef slots
#endif
#ifdef emit
#undef emit
#endif
#include <crow.h>

#include <unordered_map>
#include <mutex>
#include <QtSql>
#include <QDebug>
#include "../MessageManager/MessageManager.h"
#include "../NotificationManager/notificationmanager.h"

// using WebsocketPtr = crow::websocket::connection*;
// using UserId = int;
// using WebsocketByIdMap = std::unordered_map<UserId, WebsocketPtr>;

class Controller
{

public:

    Controller(crow::SimpleApp& app, MessageManager& manager, NotificationManager& notifManager);
    void handleRoutes();

private:

    void handleGetMessagesFromChat();
    void handleSocket();
    void  userConnected(int userId, crow::websocket::connection* conn);
    void onSendMessage(Message message);
    void onMarkReadMessage(Message message, int readBy);
    std::string getToken(const crow::request& req); //remove from this class


    NotificationManager notifManager;
    std::mutex socketMutex;
    crow::SimpleApp& app_;
    MessageManager manager;

};

#endif // CONTROLLER_H
