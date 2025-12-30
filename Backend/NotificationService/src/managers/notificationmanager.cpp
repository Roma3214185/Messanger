#include "notificationservice/managers/notificationmanager.h"

#include "Debug_profiling.h"
#include "interfaces/IRabitMQClient.h"
#include "NetworkFacade.h"
#include "interfaces/IConfigProvider.h"
#include "notificationservice/managers/socketmanager.h"

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
                                         SocketsManager* sock_manager,
                                         NetworkFacade& network_facade,
                                         IConfigProvider* provider)
    : mq_client_(mq_client), socket_manager_(sock_manager)
    , network_facade_(network_facade), provider_(provider) {
  subscribeMessageSaved();
}

void NotificationManager::subscribeMessageSaved() {
  const auto& rout_config = provider_->routes();
  SubscribeRequest request;
  request.queue = rout_config.messageSavedQueue;
  request.exchange = rout_config.exchange;
  request.routing_key = rout_config.messageSaved;
  request.exchange_type = rout_config.exchangeType;

  mq_client_->subscribe(request,
                        [this, rout_config](const std::string& event, const std::string& payload){
                          LOG_INFO("Subscribe save message received message to save: event {} and payload {}", event, payload);
                          if (event == rout_config.messageSaved)
                            handleMessageSaved(payload);
                          //else if(event == rout_config.messageStatusSaved)
                          //  handleMessageStatusSaved(payload);
                          else
                            LOG_ERROR("Invalid event");
                        });
}

void NotificationManager::handleMessageSaved(const std::string& payload) {
  auto parsed = parsePayload<Message>(payload);
  if (!parsed) return;
  onMessageSaved(*parsed);
}

void NotificationManager::subscribeMessageDeleted() {
  const auto& rout_config = provider_->routes();
  SubscribeRequest request;
  request.queue = rout_config.messageDeleted;
  request.exchange = rout_config.exchange;
  request.routing_key = rout_config.messageDeleted;
  request.exchange_type = rout_config.exchangeType;

  mq_client_->subscribe(request,
                        [this, rout_config](const std::string& event, const std::string& payload){
                          LOG_INFO("Subscribe on deleted message received message to delete: event {} and payload {}", event, payload);
                          handleOnMessageDeleted(payload);
                        });
}

void NotificationManager::handleOnMessageDeleted(const std::string& payload) {
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

void NotificationManager::notifyMessageRead(long long chat_id, const MessageStatus& status_message) {}

void NotificationManager::notifyNewMessages(Message& message, long long user_id) {}

void NotificationManager::deleteConnections(const SocketPtr& socket) {
  socket_manager_->deleteConnections(socket);
}

void NotificationManager::userConnected(long long user_id, SocketPtr socket) {
  socket_manager_->saveConnections(user_id, socket);
  // notify users who communicate with this user
}

void NotificationManager::onMarkReadMessage(Message& message, long long read_by) {
  const MessageStatus message_status{.message_id  = message.id,
                                     .receiver_id = read_by,
                                     .is_read     = true};

  // manager.saveMessageStatus(status);
  notifyMessageRead(message.id, message_status);
}

void NotificationManager::onSendMessage(Message& message) {
  LOG_INFO("Send message from '{}': {}",
           message.sender_id, nlohmann::json(message).dump());

  auto to_save     = nlohmann::json(message);
  to_save["event"] = "save_message";

  mq_client_->publish(PublishRequest{  // TODO: Factory(?)
    .exchange = provider_->routes().exchange,
    .routing_key = provider_->routes().saveMessage,
    .message = to_save.dump(),
    .exchange_type = provider_->routes().exchangeType
  });
}

void NotificationManager::onMessageStatusSaved() {}

void NotificationManager::onMessageSaved(Message& saved_message) {
  auto members_of_chat = fetchChatMembers(saved_message.chat_id);
  LOG_INFO("For chat id '{}' finded '{}' members", saved_message.chat_id, members_of_chat.size());
  LOG_INFO("Received saved message {}", nlohmann::json(saved_message).dump());

  for (auto user_id : members_of_chat) {
    LOG_INFO("{} is member of chat {}", user_id, saved_message.chat_id);
    saveDeliveryStatus(saved_message, user_id);
    notifyMember(user_id, saved_message, "new_message");
  }
}

void NotificationManager::saveMessageStatus(MessageStatus& status) {
  mq_client_->publish(PublishRequest{
    .exchange = provider_->routes().exchange,
    .routing_key = provider_->routes().saveMessageStatus,
    .message = nlohmann::json(status).dump(),
    .exchange_type = provider_->routes().exchangeType
  });
}

void NotificationManager::onUserSaved() {}

std::vector<UserId> NotificationManager::fetchChatMembers(long long chat_id) {
  return network_facade_.chat().getMembersOfChat(chat_id);
}

bool NotificationManager::notifyMember(long long user_id, const Message& msg, const std::string& type) { //todo: implement map
  auto socket = socket_manager_->getUserSocket(user_id);

  if (!socket) {
    LOG_INFO("User {} offline", user_id);
    return false;
  }
  LOG_INFO("User {} online, send message: {}", user_id, msg.text);

  auto json_message = nlohmann::json(msg);
  json_message["type"] = type;

  socket->send_text(json_message.dump());
  LOG_INFO("Sent message {} to user {}", msg.id, user_id);

  return true;
}

void NotificationManager::saveDeliveryStatus(const Message& msg, long long receiver_id) {
  LOG_INFO("saveDeliveryStatus message {} for receiver id {}", nlohmann::json(msg).dump(), receiver_id);
  MessageStatus status;
  status.message_id  = msg.id;
  status.receiver_id = receiver_id;
  status.is_read     = false;

  LOG_INFO("saveDeliveryStatus message_status {} for receiver id {}", nlohmann::json(status).dump(), receiver_id);
  saveMessageStatus(status);
}
