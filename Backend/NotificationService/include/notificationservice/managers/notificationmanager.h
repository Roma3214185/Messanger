#ifndef BACKEND_NOTIFICATIONSERVICE_NOTIFICATIONMANAGER_NOTIFICATIONMANAGER_H_
#define BACKEND_NOTIFICATIONSERVICE_NOTIFICATIONMANAGER_NOTIFICATIONMANAGER_H_

#include "entities/Message.h"
#include "entities/MessageStatus.h"
#include "entities/Reaction.h"

class NetworkFacade;
class IRabitMQClient;
class ISocket;
class IConfigProvider;
class SocketsManager;

class NotificationManager {
  IRabitMQClient *mq_client_;
  SocketsManager *socket_manager_;
  NetworkFacade &network_facade_;

 public:
  using SocketPtr = std::shared_ptr<ISocket>;

  NotificationManager(IRabitMQClient *mq_client, SocketsManager *sock_manager, NetworkFacade &network_facade);

  void saveConnections(const SocketPtr &conn);
  void deleteConnections(const SocketPtr &conn);
  virtual void userConnected(long long user_id, SocketPtr conn);
  void saveMessageStatus(MessageStatus &status);
  void saveDeliveryStatus(const Message &msg, long long receiver_id);
  bool notifyMember(long long user_id, nlohmann::json json_message, const std::string type);
  virtual void onSendMessage(Message &message);
  virtual void onMessageStatusSaved(const std::string &payload);
  virtual void onMessageSaved(const std::string &payload);
  void subscribeMessageDeleted();
  void handleOnMessageDeleted(const std::string &payload);
  void saveReaction(const Reaction& reaction);
  void deleteReaction(const Reaction& reaction);
  void subscribeMessageReactionDeleted();
  void subscribeMessageReactionSaved();
  void onMessageReactionDeleted(const std::string& payload);  //todo: make handlers also polymorhic
  void onMessageReactionSaved(const std::string& payload);

 protected:
  std::vector<long long> fetchChatMembers(long long chat_id);
  void subscribeMessageSaved();
  void subscribeMessageStatusSaved();
  std::optional<long long> getChatIdOfMessage(long long message_id);
};

#endif  // BACKEND_NOTIFICATIONSERVICE_NOTIFICATIONMANAGER_NOTIFICATIONMANAGER_H_
