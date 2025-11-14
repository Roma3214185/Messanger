#ifndef BACKEND_NOTIFICATIONSERVICE_NOTIFICATIONMANAGER_NOTIFICATIONMANAGER_H_
#define BACKEND_NOTIFICATIONSERVICE_NOTIFICATIONMANAGER_NOTIFICATIONMANAGER_H_

#include "interfaces/IRabitMQClient.h"
#include "SocketManager.h"
#include "entities/Message.h"
#include "entities/MessageStatus.h"
#include "ProdConfigProvider.h"

class NetworkFacade;
class IRabitMQClient;
class ISocket;
class IConfigProvider;

class NotificationManager {
  IRabitMQClient* mq_client_;
  SocketsManager& socket_manager_;
  NetworkFacade& network_facade_;
  IConfigProvider* provider_;

 public:
  NotificationManager(IRabitMQClient* mq_client,
                      SocketsManager& sock_manager,
                      NetworkFacade& network_facade,
                      IConfigProvider* provider = &ProdConfigProvider::instance());
  void notifyMessageRead(int chat_id, const MessageStatus& message_status);
  void notifyNewMessages(Message& message, int user_id);
  void deleteConnections(ISocket* conn);
  void userConnected(int user_id, ISocket* conn);
  void onMarkReadMessage(Message& message, int read_by);
  void onSendMessage(Message& message);
  void onMessageStatusSaved();
  void onMessageSaved(Message& message);
  void sendMessageToUser(int user_id, Message& message);
  // void saveMessage(Message& message);
  void saveMessageStatus(MessageStatus& message);
  void onUserSaved();
  QVector<UserId> fetchChatMembers(int chat_id);
  void saveDeliveryStatus(const Message& msg, int receiver_id);
  bool notifyMember(int user_id, const Message& msg);

 private:
  void subscribeMessageSaved();
  void handleMessageSaved(const std::string& payload);
};

#endif  // BACKEND_NOTIFICATIONSERVICE_NOTIFICATIONMANAGER_NOTIFICATIONMANAGER_H_
