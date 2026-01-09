#include "notificationservice/managers/notificationmanager.h"

#include "Debug_profiling.h"
#include "NetworkFacade.h"
#include "interfaces/IRabitMQClient.h"
#include "notificationservice/managers/socketmanager.h"
#include "config/Routes.h"

namespace {

template <typename T>
std::optional<T> parsePayload(const std::string &payload) {
  try {
    nlohmann::json parsed = nlohmann::json::parse(payload);
    return parsed.get<T>();
  } catch (const std::exception &e) {
    LOG_ERROR("Failed to parse message payload: {}", e.what());
    return std::nullopt;
  }
}

}  // namespace

NotificationManager::NotificationManager(IRabitMQClient *mq_client, SocketsManager *sock_manager,
                                         NetworkFacade &network_facade)
    : mq_client_(mq_client), socket_manager_(sock_manager), network_facade_(network_facade) {
  subscribeMessageSaved();
  subscribeMessageDeleted();
  subscribeMessageStatusSaved();
}

void NotificationManager::subscribeMessageSaved() {
  SubscribeRequest request;
  request.queue = Config::Routes::messageSavedQueue;
  request.exchange = Config::Routes::exchange;
  request.routing_key = Config::Routes::messageSaved;
  request.exchange_type = Config::Routes::exchangeType;

  mq_client_->subscribe(request, [this](const std::string &event, const std::string &payload) {
    LOG_INFO(
        "Subscribe save message received message to "
        "save: event {} and payload {}",
        event, payload);
    onMessageSaved(payload);
  });
}

void NotificationManager::subscribeMessageDeleted() {
  SubscribeRequest request;
  request.queue = Config::Routes::messageDeleted;
  request.exchange = Config::Routes::exchange;
  request.routing_key = Config::Routes::messageDeleted;
  request.exchange_type = Config::Routes::exchangeType;

  mq_client_->subscribe(request, [this](const std::string &event, const std::string &payload) {
    LOG_INFO(
        "Subscribe on deleted message received "
        "message to delete: event {} and payload {}",
        event, payload);
    handleOnMessageDeleted(payload);
  });
}

void NotificationManager::subscribeMessageStatusSaved() {
  SubscribeRequest request;
  request.queue = Config::Routes::messageStatusSaved;
  request.exchange = Config::Routes::exchange;
  request.routing_key = Config::Routes::messageStatusSaved;
  request.exchange_type = Config::Routes::exchangeType;

  mq_client_->subscribe(request, [this](const std::string &event, const std::string &payload) {
    LOG_INFO("Subscribe on message status saved received: event {} and payload {}", event, payload);
    onMessageStatusSaved(payload);
  });
}

void NotificationManager::handleOnMessageDeleted(const std::string &payload) {
  auto parsed = parsePayload<Message>(payload);
  if (!parsed) return;

  Message deleteted_message = *parsed;
  auto members_of_chat = fetchChatMembers(deleteted_message.chat_id);
  LOG_INFO("For chat id '{}' finded '{}' members", deleteted_message.chat_id, members_of_chat.size());
  LOG_INFO("Received deleted message {}", nlohmann::json(deleteted_message).dump());

  for (auto user_id : members_of_chat) {
    LOG_INFO("{} is member of chat {}", user_id, deleteted_message.chat_id);
    notifyMember(user_id, deleteted_message, "delete_message");
  }
}

void NotificationManager::deleteConnections(const SocketPtr &socket) { socket_manager_->deleteConnections(socket); }

void NotificationManager::userConnected(long long user_id, SocketPtr socket) {
  socket_manager_->saveConnections(user_id, socket);
  // notify users who communicate with this user
}

void NotificationManager::onSendMessage(Message &message) {
  LOG_INFO("Send message from '{}': {}", message.sender_id, nlohmann::json(message).dump());

  auto to_save = nlohmann::json(message);
  to_save["event"] = "save_message";

  mq_client_->publish(PublishRequest{// TODO: Factory(?)
                                     .exchange = Config::Routes::exchange,
                                     .routing_key = Config::Routes::saveMessage,
                                     .message = to_save.dump(),
                                     .exchange_type = Config::Routes::exchangeType});
}

void NotificationManager::onMessageStatusSaved(const std::string &payload) {
  auto parsed = parsePayload<MessageStatus>(payload);
  if (!parsed) return;

  MessageStatus message_status = *parsed;
  // get chat_id by message_id;
  std::optional<long long> chat_id = getChatIdOfMessage(message_status.message_id);
  if (!chat_id.has_value()) {
    LOG_ERROR("Chat id for message {} not founded", message_status.message_id);
    return;
  }
  auto members_of_chat = fetchChatMembers(*chat_id);
  LOG_INFO("For chat id '{}' finded '{}' members", *chat_id, members_of_chat.size());
  LOG_INFO("Received saved message status {}", nlohmann::json(message_status).dump());

  for (auto user_id : members_of_chat) {
    LOG_INFO("{} is member of chat {}", user_id, *chat_id);
    notifyMember(user_id, message_status, "read_message");
  }
}

void NotificationManager::onMessageSaved(const std::string &payload) {
  auto parsed = parsePayload<Message>(payload);
  if (!parsed) return;
  Message saved_message = *parsed;

  auto members_of_chat = fetchChatMembers(saved_message.chat_id);
  LOG_INFO("For chat id '{}' finded '{}' members", saved_message.chat_id, members_of_chat.size());
  LOG_INFO("Received saved message {}", nlohmann::json(saved_message).dump());

  for (auto user_id : members_of_chat) {
    LOG_INFO("{} is member of chat {}", user_id, saved_message.chat_id);
    saveDeliveryStatus(saved_message,
                       user_id);  // todo: this must be in message service when i save message
    notifyMember(user_id, saved_message, "new_message");
  }
}

void NotificationManager::saveMessageStatus(MessageStatus &status) {
  mq_client_->publish(PublishRequest{.exchange = Config::Routes::exchange,
                                     .routing_key = Config::Routes::saveMessageStatus,
                                     .message = nlohmann::json(status).dump(),
                                     .exchange_type = Config::Routes::exchangeType});
}

std::vector<UserId> NotificationManager::fetchChatMembers(long long chat_id) {
  return network_facade_.chat().getMembersOfChat(chat_id);
}

bool NotificationManager::notifyMember(long long user_id, nlohmann::json json_message,
                                       const std::string type) {  // todo: implement enum
  auto socket = socket_manager_->getUserSocket(user_id);

  if (!socket) {
    LOG_INFO("User {} offline", user_id);
    return false;
  }

  LOG_INFO("User {} online, type {} and send message: {}", user_id, type, json_message.dump());
  json_message["type"] = std::move(type);
  socket->send_text(json_message.dump());
  LOG_INFO("Sent message {} to user {}", json_message.dump(), user_id);

  return true;
}

void NotificationManager::saveDeliveryStatus(const Message &msg,
                                             long long receiver_id) {  // todo: delete this
  LOG_INFO("saveDeliveryStatus message {} for receiver id {}", nlohmann::json(msg).dump(), receiver_id);
  MessageStatus status;
  status.message_id = msg.id;
  status.receiver_id = receiver_id;
  status.is_read = false;

  LOG_INFO("saveDeliveryStatus message_status {} for receiver id {}", nlohmann::json(status).dump(), receiver_id);
  saveMessageStatus(status);
}

std::optional<long long> NotificationManager::getChatIdOfMessage(long long message_id) {
  DBC_REQUIRE(message_id > 0);
  return network_facade_.msg().getChatIdOfMessage(message_id);
}
