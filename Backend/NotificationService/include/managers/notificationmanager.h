#ifndef BACKEND_NOTIFICATIONSERVICE_NOTIFICATIONMANAGER_NOTIFICATIONMANAGER_H_
#define BACKEND_NOTIFICATIONSERVICE_NOTIFICATIONMANAGER_NOTIFICATIONMANAGER_H_

#include "entities/Message.h"
#include "entities/MessageStatus.h"
#include "RabbitMQClient/RabbitMQClient.h"
#include "SocketManager.h"

class NetworkManager;
class RabbitMQClient;

class NotificationManager {
  RabbitMQClient& mq_client_;
  SocketsManager& socket_manager_;
  NetworkManager& network_manager_;

 public:
  NotificationManager(RabbitMQClient& mq_client, SocketsManager& sock_manager,
                      NetworkManager& network_manager);
  void notifyMessageRead(int chat_id, const MessageStatus& message_status);
  void notifyNewMessages(Message& message, int user_id);
  void saveConnections(int user_id, WebsocketPtr socket);
  void deleteConnections(WebsocketPtr conn);
  void userConnected(int user_id, WebsocketPtr conn);
  void onMarkReadMessage(Message& message, int read_by);
  void onSendMessage(Message& message);
  void onMessageStatusSaved();
  void onMessageSaved(Message& message);
  void sendMessageToUser(int user_id, Message& message);
  //void saveMessage(Message& message);
  void saveMessageStatus(MessageStatus& message);
  void onUserSaved();

 private:
  void subscribeMessageSaved();
  void handleMessageSaved(const std::string& payload);
};

#endif  // BACKEND_NOTIFICATIONSERVICE_NOTIFICATIONMANAGER_NOTIFICATIONMANAGER_H_
