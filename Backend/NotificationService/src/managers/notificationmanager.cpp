#include "managers/notificationmanager.h"

#include "Debug_profiling.h"
#include "interfaces/IRabitMQClient.h"
#include "managers/SocketManager.h"
#include "NetworkFacade.h"
#include "interfaces/ISocket.h"
#include "interfaces/IConfigProvider.h"

namespace {

template <typename T>
std::optional<T> parsePayload(const std::string& payload) {
  try {
    nlohmann::json parsed = nlohmann::json::parse(payload);
    return parsed.get<T>();
  }
  catch (const std::exception& e) {
    LOG_ERROR("Failed to parse message payload: {}", e.what());
    return std::nullopt;
  }
}

}  //namespace

NotificationManager::NotificationManager(IRabitMQClient* mq_client,
                                         SocketsManager& sock_manager,
                                         NetworkFacade& network_facade,
                                         IConfigProvider* provider)
    : mq_client_(mq_client), socket_manager_(sock_manager)
    , network_facade_(network_facade), provider_(provider) {
  subscribeMessageSaved();
}

void NotificationManager::subscribeMessageSaved() {
  const auto& rout_config = provider_->routes();
  SubscribeRequest request;
  request.queue = rout_config.notificationQueue;
  request.exchange = rout_config.exchange;
  request.routingKey = rout_config.messageSaved;

  mq_client_->subscribe(request,
                        [this, rout_config](const std::string& event, const std::string& payload){
                          if (event == rout_config.messageSaved)
                            handleMessageSaved(payload);
                          else
                            LOG_ERROR("Invalid event");
                        });
}

void NotificationManager::handleMessageSaved(const std::string& payload) {
  auto parsed = parsePayload<Message>(payload);
  if (!parsed) return;

  Message saved_message = *parsed;

  LOG_INFO("Received saved message id {} text '{}'", saved_message.id, saved_message.text);

  auto chat_members = fetchChatMembers(saved_message.chat_id);

  for (auto user_id : chat_members) {
    if (!notifyMember(user_id, saved_message))
      continue;

    saveDeliveryStatus(saved_message, user_id);
  }
}

void NotificationManager::notifyMessageRead(int chat_id, const MessageStatus& status_message) {}

void NotificationManager::notifyNewMessages(Message& message, int user_id) {}

void NotificationManager::deleteConnections(ISocket* socket) {
  socket_manager_.deleteConnections(socket);
}

void NotificationManager::userConnected(int user_id, ISocket* socket) {
  socket_manager_.saveConnections(user_id, socket);
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
  LOG_INFO("Send message from '{}' to chatId '{}' (text: '{}')",
           message.sender_id, message.chat_id, message.text);

  auto to_save     = nlohmann::json(message);
  to_save["event"] = "save_message";

  PublishRequest request;  // TODO: Factory(?)
  request.exchange = provider_->routes().exchange;
  request.routingKey = provider_->routes().saveMessage;
  request.message = to_save.dump();

  mq_client_->publish(request);
}

void NotificationManager::onMessageStatusSaved() {}

void NotificationManager::onMessageSaved(Message& message) {
  auto members_of_chat = network_facade_.chat().getMembersOfChat(message.chat_id);

  LOG_INFO("Message('{}') is saved with id '{}'", message.text, message.id);
  LOG_INFO("For chat id '{}' finded '{}' members", message.chat_id, members_of_chat.size());

  for (auto toUser : std::as_const(members_of_chat)) {
    MessageStatus messageStatus{.message_id = message.id, .receiver_id = toUser, .is_read = false};

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

  user_socket->send_text(nlohmann::json(message).dump());
}

void NotificationManager::saveMessageStatus(MessageStatus& status) {
  PublishRequest request;
  request.exchange = provider_->routes().exchange;
  request.routingKey = provider_->routes().saveMessageStatus;
  request.message = nlohmann::json(status).dump();

  mq_client_->publish(request);
}

void NotificationManager::onUserSaved() {}

QVector<UserId> NotificationManager::fetchChatMembers(int chat_id) {
  return network_facade_.chat().getMembersOfChat(chat_id);
}

bool NotificationManager::notifyMember(int user_id, const Message& msg) {
  auto* socket = socket_manager_.getUserSocket(user_id);

  if (!socket) {
    LOG_INFO("User {} offline", user_id);
    return false;
  }

  auto json_message = to_crow_json(msg);
  json_message["type"] = "new_message";

  socket->send_text(json_message.dump());
  LOG_INFO("Sent message {} to user {}", msg.id, user_id);

  return true;
}

void NotificationManager::saveDeliveryStatus(const Message& msg, int receiver_id) {
  MessageStatus status;
  status.message_id  = msg.id;
  status.receiver_id = receiver_id;
  status.is_read     = false;

  saveMessageStatus(status);
}
