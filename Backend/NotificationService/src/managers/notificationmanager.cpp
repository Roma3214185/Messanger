#include "managers/notificationmanager.h"

#include "Debug_profiling.h"
#include "interfaces/IRabitMQClient.h"
#include "managers/SocketManager.h"
#include "managers/networkmanager.h"

const std::string kMessageSaved      = "message_saved";
const std::string kSaveMessage       = "save_message";
const std::string kSaveMessageStatus = "save_message_status";
const std::string kNotificationQueue = "notification_service_queue";
const std::string kExchange          = "app.events";

NotificationManager::NotificationManager(IRabitMQClient* mq_client,
                                         SocketsManager& sock_manager,
                                         NetworkManager* network_manager)
    : mq_client_(mq_client), socket_manager_(sock_manager)
    , network_manager_(network_manager) {
  subscribeMessageSaved();
}

void NotificationManager::subscribeMessageSaved() {
  mq_client_->subscribe(
      kNotificationQueue,
      kExchange,
      kMessageSaved,
      [this](const std::string& event, const std::string& payload) {
        if (event == kMessageSaved)
          handleMessageSaved(payload);
        else
          LOG_ERROR("Invalid event");
      },
      "topic");
}

void NotificationManager::handleMessageSaved(const std::string& payload) {
  nlohmann::json parsed;
  try {
    parsed = nlohmann::json::parse(payload);
  } catch (const std::exception& e) {
    LOG_ERROR("Failed to parse message payload: {}", e.what());
    return;
  }

  auto saved_message = parsed.get<Message>();

  LOG_INFO("Received saved message id {} text '{}'", saved_message.id, saved_message.text);

  auto chat_members = network_manager_->getMembersOfChat(saved_message.chat_id);
  for (auto user_id : chat_members) {
    auto* socket = socket_manager_.getUserSocket(user_id);
    if (!socket) {
      LOG_INFO("User {} offline", user_id);
      continue;
    }

    auto json_message    = to_crow_json(saved_message);
    json_message["type"] = "new_message";
    socket->send_text(json_message.dump());
    LOG_INFO("Sent message {} to user {}", saved_message.id, user_id);

    MessageStatus status;
    status.message_id  = saved_message.id;
    status.receiver_id = user_id;
    status.is_read     = false;

    saveMessageStatus(status);
  }
}

void NotificationManager::notifyMessageRead(int chat_id, const MessageStatus& status_message) {}

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
  const MessageStatus message_status{.message_id  = message.id,
                                     .receiver_id = read_by,
                                     .is_read     = true,
                                     .read_at     = QDateTime::currentSecsSinceEpoch()};

  // manager.saveMessageStatus(status);
  notifyMessageRead(message.id, message_status);
}

void NotificationManager::onSendMessage(Message& message) {
  PROFILE_SCOPE("Controller::onSendMessage");
  LOG_INFO("Send message from '{}' to chatId '{}' (text: '{}')",
           message.sender_id,
           message.chat_id,
           message.text);

  auto to_save     = nlohmann::json(message);
  to_save["event"] = "save_message";
  mq_client_->publish(kExchange, kSaveMessage, to_save.dump());
}

void NotificationManager::onMessageStatusSaved() {}

void NotificationManager::onMessageSaved(Message& message) {
  auto members_of_chat = network_manager_->getMembersOfChat(message.chat_id);

  LOG_INFO("Message('{}') is saved with id '{}'", message.text, message.id);
  LOG_INFO("For chat id '{}' finded '{}' members", message.chat_id, members_of_chat.size());

  for (auto toUser : std::as_const(members_of_chat)) {
    LOG_INFO("Chat id: '{}'; member is ", message.chat_id, toUser);

    MessageStatus messageStatus{.message_id = message.id, .receiver_id = toUser, .is_read = false};

    qDebug() << "For text " << message.text << " local_id " << message.local_id;
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

  auto json_message = nlohmann::json(message);
  user_socket->send_text(json_message.dump());
}

void NotificationManager::saveMessageStatus(MessageStatus& status) {
  auto status_json = nlohmann::json(status);

  mq_client_->publish(kExchange, kSaveMessageStatus, status_json.dump());
}

void NotificationManager::onUserSaved() {}
