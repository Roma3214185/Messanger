#ifndef BACKEND_NOTIFICATIONSERVICE_NOTIFICATIONMANAGER_NOTIFICATIONMANAGER_H_
#define BACKEND_NOTIFICATIONSERVICE_NOTIFICATIONMANAGER_NOTIFICATIONMANAGER_H_

#include "interfaces/IRabitMQClient.h"
#include "entities/Message.h"
#include "entities/MessageStatus.h"
#include "ProdConfigProvider.h"

class NetworkFacade;
class IRabitMQClient;
class ISocket;
class IConfigProvider;
class SocketsManager;

class NotificationManager {
  IRabitMQClient* mq_client_;
  SocketsManager* socket_manager_;
  NetworkFacade& network_facade_;
  IConfigProvider* provider_;

 public:
  using SocketPtr = std::shared_ptr<ISocket>;

  NotificationManager(IRabitMQClient* mq_client,
                      SocketsManager* sock_manager,
                      NetworkFacade& network_facade,
                      IConfigProvider* provider = &ProdConfigProvider::instance());
  void notifyMessageRead(int chat_id, const MessageStatus& message_status);
  void notifyNewMessages(Message& message, int user_id);
  void deleteConnections(SocketPtr conn);
  virtual void userConnected(int user_id, SocketPtr conn);
  void saveMessageStatus(MessageStatus& message);
  void saveDeliveryStatus(const Message& msg, int receiver_id);
  bool notifyMember(int user_id, const Message& msg);
  virtual void onMarkReadMessage(Message& message, int read_by);
  virtual void onSendMessage(Message& message);
  virtual void onMessageStatusSaved();
  virtual void onMessageSaved(Message& message);
  virtual void onUserSaved();
  virtual void handleMessageSaved(const std::string& payload);

 protected:
  QVector<int> fetchChatMembers(int chat_id);
  void subscribeMessageSaved();
};

#endif  // BACKEND_NOTIFICATIONSERVICE_NOTIFICATIONMANAGER_NOTIFICATIONMANAGER_H_
