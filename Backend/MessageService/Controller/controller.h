#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <mutex>
#include <unordered_map>
#include <QtSql>

#include <crow.h>

#include "Message.h"
#include "notificationmanager.h"
#include "rabbitmqclient.h"

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
