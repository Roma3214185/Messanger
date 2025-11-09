#include "managers/notificationmanager.h"

#include "entities/MessageStatus.h"
#include "managers/NetworkManager.h"

void NotificationManager::notifyMessageRead(int chat_id, const MessageStatus& status) {
  auto members = NetworkManager::getMembersOfChat(chat_id);
  for (int userId : members) {
    if (user_sockets_.find(userId) == user_sockets_.end()) {
      crow::json::wvalue msgJson;
      msgJson["type"]       = "message_read";
      msgJson["message_id"] = status.message_id;
      msgJson["reader_id"]  = status.receiver_id;

      user_sockets_[userId]->send_text(msgJson.dump());
    }
  }
}

void NotificationManager::notifyNewMessages(Message& message, int receiver_id) {
  PROFILE_SCOPE("NotificationManager::NotifyNewMessages");
  LOG_INFO("Chat id: '{}'; member is ", message.chat_id, receiver_id);
  auto iter = user_sockets_.find(receiver_id);

  if (iter != user_sockets_.end()) {
    auto forwardMsg    = to_crow_json(message);
    forwardMsg["type"] = "message";
    iter->second->send_text(forwardMsg.dump());  // don;t use dump??
    LOG_INFO("Forward message to id '{}'", receiver_id);
  } else {
    LOG_INFO("User offline, Message is saved to id '{}'", receiver_id);
  }
}
