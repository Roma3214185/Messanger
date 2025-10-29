#include "notificationmanager.h"

#include "socketmanager.h"
#include "networkmanager.h"
#include "rabbitmqclient.h"
#include "Debug_profiling.h"

const std::string kSavingMessageSaved = "message_saved";

NotificationManager::NotificationManager(RabbitMQClient& mq_client,
                                         SocketsManager& sock_manager,
                                         NetworkManager& network_manager)
    : mq_client_(mq_client), socket_manager_(sock_manager), network_manager_(network_manager) {
  subscribeMessageSaved();
}

void NotificationManager::subscribeMessageSaved(){
  mq_client_.subscribe(
      "app.events_queue",
      "app.events",
      kSavingMessageSaved,
      [this](const std::string& event, const std::string& payload) {
        if (event == kSavingMessageSaved) {
          nlohmann::json parsed = nlohmann::json::parse(payload);
          Message saved_message;
          from_json(parsed, saved_message);
          LOG_INFO("Received saved message with id {} and text {}", saved_message.id, saved_message.text);
          auto chats_ids = network_manager_.getMembersOfChat(saved_message.chat_id);

          for (auto user_id: chats_ids) {
            auto* socket = socket_manager_.getUserSocket(user_id);
            if (!socket) {
              LOG_ERROR("User {} is offline", user_id);
              continue;
            }
            socket->send_text(payload);

            // TODO(roma): Save message_status
          }
        }
      }
      );
}

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
  mq_client_.publish("app.events", "save_message", to_save.dump());
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
