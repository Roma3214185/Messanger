#include "notificationmanager.h"

#include <json/json.h>

#include "socketmanager.h"
#include "networkmanager.h"
#include "rabbitmqclient.h"
#include "Debug_profiling.h"

namespace {

inline void to_json(nlohmann::json& j, const Message& m) {
  j = nlohmann::json{{"id", m.id},
                     {"chat_id", m.chat_id},
                     {"sender_id", m.sender_id},
                     {"text", m.text},
                     {"timestamp", m.timestamp}};
}

inline void from_json(const nlohmann::json& j, Message& u) {
  j.at("id").get_to(u.id);
  j.at("chat_id").get_to(u.chat_id);
  j.at("sender_id").get_to(u.sender_id);
  j.at("text").get_to(u.text);
  j.at("timestamp").get_to(u.timestamp);
}

}  // namespace


NotificationManager::NotificationManager(RabbitMQClient& mq_client,
                                         SocketsManager& sock_manager,
                                         NetworkManager& network_manager)
    : mq_client_(mq_client), socket_manager_(sock_manager), network_manager_(network_manager) {}

void NotificationManager::notifyMessageRead(int chat_id,
                                            const MessageStatus& status_message) {}

void NotificationManager::notifyNewMessages(Message& message, int user_id) {}

void NotificationManager::saveConnections(int user_id, WebsocketPtr socket) {
  socket_manager_.saveConnections(user_id, socket);
}

void NotificationManager::deleteConnections(WebsocketPtr conn) {
  socket_manager_.deleteConnections(conn);
}

void NotificationManager::userConnected(int user_id, WebsocketPtr conn) {
  saveConnections(user_id, conn);
  // notify users who communicate with this user
}

void NotificationManager::onMarkReadMessage(Message& message, int read_by) {
  const MessageStatus message_status {
      .id = message.id,
      .receiver_id = read_by,
      .is_read = true,
      .read_at = QDateTime::currentSecsSinceEpoch() };

  // manager.saveMessageStatus(status);
  notifyMessageRead(message.id, message_status);
}

void NotificationManager::onSendMessage(Message& message) {
  PROFILE_SCOPE("Controller::onSendMessage");
  LOG_INFO("Send message from '{}' to chatId '{}' (text: '{}')", message.sender_id,
           message.chat_id, message.text);

  auto to_save = nlohmann::json{{"event", "save_message"}};
  to_json(to_save, message);
  mq_client_.publish("app.events", "save", to_save.dump());

  // sendMessage(mq, message);

  // mq_client_.subscribe("notification_service.in", [this](const std::string& body) {
  //   auto res = nlohmann::json::parse(body);
  //   if (res["event"] == "saved") {
  //     LOG_INFO("Saved accept from mq");
  //     if (res["saved"] == "User") {
  //       // User user = from_json();
  //       onUserSaved();
  //     } else if (res["saved"] == "Message") {
  //       Message newmessage;
  //       from_json(res, newmessage);
  //       onMessageSaved(newmessage);
  //     } else if (res["saved"] == "MessageStatus") {
  //       onMessageStatusSaved();
  //     } else {
  //       LOG_ERROR("Onknow type of saved enity");
  //     }
  //   }
  // });
}

void NotificationManager::onMessageStatusSaved() {}

void NotificationManager::onMessageSaved(Message& message) {
  auto members_of_chat = network_manager_.getMembersOfChat(message.chat_id);

  LOG_INFO("Message('{}') is saved with id '{}'", message.text, message.id);
  LOG_INFO("For chat id '{}' finded '{}' members", message.chat_id,
           members_of_chat.size());

  for (auto toUser : std::as_const(members_of_chat)) {
    LOG_INFO("Chat id: '{}'; member is ", message.chat_id, toUser);

    MessageStatus messageStatus{
        .id = message.id, .receiver_id = toUser, .is_read = false};

    saveMessageStatus(messageStatus);
    sendMessageToUser(toUser, message);
  }
}

void NotificationManager::sendMessageToUser(int user_id, Message& message) {
  auto user_socket = socket_manager_.getUserSocket(user_id);
  if (!user_socket) {
    LOG_INFO("User offline");
    return;
  }

  nlohmann::json json_message;
  to_json(json_message, message);
  user_socket->send_text(json_message.dump());
}

void NotificationManager::saveMessageStatus(MessageStatus& message) {
  auto to_save = nlohmann::json{{"event", "save_message_status"}};
  to_json(to_save, message);
  mq_client_.publish("app.events", "save", to_save.dump());
}

void NotificationManager::onUserSaved() {}
