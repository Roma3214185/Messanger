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
#include "../NotificationManager/notificationmanager.h"
#include "../Headers/Message.h"
#include "../../RabbitMQClient/rabbitmqclient.h"

class Controller
{

public:

    Controller(crow::SimpleApp& app, RabbitMQClient& mq, MessageManager& manager);
    void handleRoutes();

private:

    void handleGetMessagesFromChat();
    //void handleSocket();
    //void  userConnected(int userId, crow::websocket::connection* conn);
    void onSendMessage(Message message);
    //void onMarkReadMessage(Message message, int readBy);
    std::string getToken(const crow::request& req);

    std::mutex socketMutex;
    crow::SimpleApp& app_;
    MessageManager& manager;
    RabbitMQClient& mq;
};

#endif // CONTROLLER_H
