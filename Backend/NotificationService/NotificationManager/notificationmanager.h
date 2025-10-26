#ifndef NOTIFICATIONMANAGER_H
#define NOTIFICATIONMANAGER_H

#include "Message.h"
#include "MessageStatus.h"
#include "SocketManager.h"
#include "RabbitMQClient/rabbitmqclient.h"

class NetworkManager;
class RabbitMQClient;

class NotificationManager {
    RabbitMQClient& mq;
    SocketsManager& socketManager;
    NetworkManager& networkManager;
public:
    NotificationManager(RabbitMQClient& mq, SocketsManager& sockManager, NetworkManager& networkManager);
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
};

#endif // NOTIFICATIONMANAGER_H
