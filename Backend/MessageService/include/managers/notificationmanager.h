#ifndef BACKEND_MESSAGESERVICE_NOTIFICATIONMANAGER_NOTIFICATIONMANAGER_H_
#define BACKEND_MESSAGESERVICE_NOTIFICATIONMANAGER_NOTIFICATIONMANAGER_H_

#include <crow.h>

#include <unordered_map>

#include "entities/Message.h"

class MessageStatus;
class NetworkManager;

using WebsocketPtr     = crow::websocket::connection*;
using UserId           = int;
using WebsocketByIdMap = std::unordered_map<UserId, WebsocketPtr>;


class NotificationManager {
 public:
  NotificationManager(NetworkManager* network_manager);
  void notifyMessageRead(int chat_id, const MessageStatus& message_status);
  void notifyNewMessages(Message& message, int user_id);
  void saveConnections(int user_id, WebsocketPtr socket);
  void deleteConnections(WebsocketPtr conn);

 private:
  WebsocketByIdMap user_sockets_;
  NetworkManager* network_manager_;
};

#endif  // BACKEND_MESSAGESERVICE_NOTIFICATIONMANAGER_NOTIFICATIONMANAGER_H_
