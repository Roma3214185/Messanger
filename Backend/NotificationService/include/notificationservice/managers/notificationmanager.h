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
  void notifyMessageRead(long long chat_id, const MessageStatus& message_status);
  void notifyNewMessages(Message& message, long long user_id);
  void saveConnections(const SocketPtr& conn); //think about move these connections
  void deleteConnections(const SocketPtr& conn);
  virtual void userConnected(long long user_id, SocketPtr conn);
  void saveMessageStatus(MessageStatus& status);
  void saveDeliveryStatus(const Message& msg, long long receiver_id);
  bool notifyMember(long long user_id, const Message& msg);
  virtual void onMarkReadMessage(Message& message, long long read_by);
  virtual void onSendMessage(Message& message);
  virtual void onMessageStatusSaved();
  virtual void onMessageSaved(Message& message);
  virtual void onUserSaved();
  virtual void handleMessageSaved(const std::string& payload);

 protected:
  std::vector<long long> fetchChatMembers(long long chat_id);
  void subscribeMessageSaved();
};

#endif  // BACKEND_NOTIFICATIONSERVICE_NOTIFICATIONMANAGER_NOTIFICATIONMANAGER_H_
