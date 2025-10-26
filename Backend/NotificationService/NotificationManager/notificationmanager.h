#ifndef NOTIFICATIONMANAGER_H
#define NOTIFICATIONMANAGER_H

#include "Message.h"
#include "rabbitmqclient.h"
#include "networkmanager.h"
#include "SocketManager.h"
#include "MessageStatus.h"

class NotificationManager
{
public:
    NotificationManager(RabbitMQClient& mq, SocketsManager& sockManager, NetworkManager& networkManager);

    void init();
    void notifyMessageRead(int chatId, const MessageStatus& status);
    void notifyNewMessages(Message msg, int userId);
    void saveConnections(int userId, WebsocketPtr socket);
    void deleteConnections(WebsocketPtr conn);
    void userConnected(int userId, WebsocketPtr conn);
    void onMarkReadMessage(Message message, int readBy);
    void onSendMessage(Message msg);
    void onMessageStatusSaved();
    void onMessageSaved(Message msg);
    void sendMessageToUser(int userId, Message& msg);
    void saveMessage(Message& msg);
    void saveMessageStatus(MessageStatus& msg);
    void onUserSaved();

private:
    void handleMessage(const std::string& body);

    RabbitMQClient& mq;
    SocketsManager& socketManager;
    NetworkManager& networkManager;
};

#endif // NOTIFICATIONMANAGER_H
