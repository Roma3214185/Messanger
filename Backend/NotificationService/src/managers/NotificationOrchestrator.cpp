#include "notificationservice/managers/NotificationOrchestrator.h"

#include "Debug_profiling.h"
#include "NetworkFacade.h"
#include "config/Routes.h"
#include "interfaces/IRabitMQClient.h"
#include "utils.h"
#include "notificationservice/SocketRepository.h"
#include "notificationservice/IPublisher.h"
#include "notificationservice/ISubscriber.h"
#include "notificationservice/SocketNotifier.h"

NotificationOrchestrator::NotificationOrchestrator(INetworkFacade *network_facade,
                                                   IPublisher* publisher, INotifier* notifier)
    : network_facade_(network_facade), publisher_(publisher), notifier_(notifier) {
}

RabbitNotificationSubscriber::RabbitNotificationSubscriber(IEventSubscriber* mq_client, NotificationOrchestrator* notification_orchestrator)
    : mq_client_(mq_client), notification_orchestrator_(notification_orchestrator) { }

void RabbitNotificationSubscriber::subscribeAll() {
    subscribeMessageSaved();
    subscribeMessageDeleted();
    subscribeMessageStatusSaved();
    subscribeMessageReactionDeleted();
    subscribeMessageReactionSaved();
}

void RabbitNotificationSubscriber::subscribeMessageReactionDeleted() {
  SubscribeRequest request;
  request.queue = Config::Routes::messageReactionDeleted;
  request.exchange = Config::Routes::exchange;
  request.routing_key = Config::Routes::messageReactionDeleted;
  request.exchange_type = Config::Routes::exchangeType;

  mq_client_->subscribe(request, [this](const std::string &event, const std::string &payload) {
    LOG_INFO(
        "Subscribe save message received message to "
        "save: event {} and payload {}",
        event, payload);
    notification_orchestrator_->onMessageReactionDeleted(payload);
  });
}

void RabbitNotificationSubscriber::subscribeMessageReactionSaved() {
  SubscribeRequest request;
  request.queue = Config::Routes::messageReactionSaved;
  request.exchange = Config::Routes::exchange;
  request.routing_key = Config::Routes::messageReactionSaved;
  request.exchange_type = Config::Routes::exchangeType;

  mq_client_->subscribe(request, [this](const std::string &event, const std::string &payload) {
    LOG_INFO(
        "Subscribe save message received message to "
        "save: event {} and payload {}",
        event, payload);
    notification_orchestrator_->onMessageReactionSaved(payload);
  });
}

void NotificationOrchestrator::onMessageReactionDeleted(const std::string &payload) {
  auto parsed_reaction = utils::parsePayload<Reaction>(payload);
  if (!parsed_reaction) return;
  Reaction reaction_deleted = parsed_reaction.value();
  auto chat_id_opt = getChatIdOfMessage(reaction_deleted.message_id);
  if (!chat_id_opt.has_value()) {
    LOG_ERROR("Can't find chat_id_opt for Reaction {}", nlohmann::json(reaction_deleted).dump());
    return;
  }

  auto members_of_chat = fetchChatMembers(chat_id_opt.value());
  LOG_INFO("For chat id '{}' finded '{}' members", chat_id_opt.value(), members_of_chat.size());
  LOG_INFO("Received deleted reaction {}", nlohmann::json(reaction_deleted).dump());

  for (auto user_id : members_of_chat) {
    LOG_INFO("{} is member of chat {}", user_id, chat_id_opt.value());
    notifier_->notifyMember(user_id, reaction_deleted, "delete_reaction");
  }
}

void NotificationOrchestrator::onMessageReactionSaved(const std::string &payload) {
  auto parsed_reaction = utils::parsePayload<Reaction>(payload);
  if (!parsed_reaction) return;
  Reaction reaction_saved = parsed_reaction.value();
  auto chat_id_opt = getChatIdOfMessage(reaction_saved.message_id);
  if (!chat_id_opt.has_value()) {
    LOG_ERROR("Can't find chat_id_opt for Reaction {}", nlohmann::json(reaction_saved).dump());
    return;
  }

  auto members_of_chat = fetchChatMembers(chat_id_opt.value());
  LOG_INFO("For chat id '{}' finded '{}' members", chat_id_opt.value(), members_of_chat.size());
  LOG_INFO("Received saved reaction {}", nlohmann::json(reaction_saved).dump());

  for (auto user_id : members_of_chat) {
    LOG_INFO("{} is member of chat {}", user_id, chat_id_opt.value());
    notifier_->notifyMember(user_id, reaction_saved, "save_reaction");
  }
}

void RabbitNotificationSubscriber::subscribeMessageSaved() {
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
    notification_orchestrator_->onMessageSaved(payload);
  });
}

void RabbitNotificationSubscriber::subscribeMessageDeleted() {
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
    notification_orchestrator_->onMessageDeleted(payload);
  });
}

void RabbitNotificationSubscriber::subscribeMessageStatusSaved() {
  SubscribeRequest request;
  request.queue = Config::Routes::messageStatusSaved;
  request.exchange = Config::Routes::exchange;
  request.routing_key = Config::Routes::messageStatusSaved;
  request.exchange_type = Config::Routes::exchangeType;

  mq_client_->subscribe(request, [this](const std::string &event, const std::string &payload) {
    LOG_INFO("Subscribe on message status saved received: event {} and payload {}", event, payload);
    notification_orchestrator_->onMessageStatusSaved(payload);
  });
}

void NotificationOrchestrator::onMessageDeleted(const std::string &payload) {
  auto parsed = utils::parsePayload<Message>(payload);
  if (!parsed) return;

  Message deleteted_message = *parsed;
  auto members_of_chat = fetchChatMembers(deleteted_message.chat_id);
  LOG_INFO("For chat id '{}' finded '{}' members", deleteted_message.chat_id, members_of_chat.size());
  LOG_INFO("Received deleted message {}", nlohmann::json(deleteted_message).dump());

  for (auto user_id : members_of_chat) {
    LOG_INFO("{} is member of chat {}", user_id, deleteted_message.chat_id);
    notifier_->notifyMember(user_id, deleteted_message, "delete_message");
  }
}

// void NotificationOrchestrator::onUserConnected(long long user_id, SocketPtr socket) {
//   socket_manager_->saveConnections(user_id, std::move(socket));
//   // notify users who communicate with this user
// }

void RabbitNotificationPublisher::saveMessage(const Message &message) {
  LOG_INFO("Send message from '{}': {}", message.sender_id, nlohmann::json(message).dump());

  auto to_save = nlohmann::json(message);
  to_save["event"] = "save_message";

  mq_client_->publish(PublishRequest{// TODO: Factory(?)
                                     .exchange = Config::Routes::exchange,
                                     .routing_key = Config::Routes::saveMessage,
                                     .message = to_save.dump(),
                                     .exchange_type = Config::Routes::exchangeType});
}

void NotificationOrchestrator::onMessageStatusSaved(const std::string &payload) {
  auto parsed = utils::parsePayload<MessageStatus>(payload);
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
    notifier_->notifyMember(user_id, message_status, "read_message");
  }
}

void NotificationOrchestrator::onMessageSaved(const std::string &payload) {
  auto parsed = utils::parsePayload<Message>(payload);
  if (!parsed) return;
  Message saved_message = *parsed;

  auto members_of_chat = fetchChatMembers(saved_message.chat_id);
  LOG_INFO("For chat id '{}' finded '{}' members", saved_message.chat_id, members_of_chat.size());
  LOG_INFO("Received saved message {}", nlohmann::json(saved_message).dump());

  for (auto user_id : members_of_chat) {
    LOG_INFO("{} is member of chat {}", user_id, saved_message.chat_id);
    publisher_->saveDeliveryStatus(saved_message,
                       user_id);  // todo: this must be in message service when i save message
    notifier_->notifyMember(user_id, saved_message, "new_message");
  }
}

RabbitNotificationPublisher::RabbitNotificationPublisher(IEventPublisher* mq_client) : mq_client_(mq_client) { }

void RabbitNotificationPublisher::saveMessageStatus(MessageStatus &status) {
  mq_client_->publish(PublishRequest{.exchange = Config::Routes::exchange,
                                     .routing_key = Config::Routes::saveMessageStatus,
                                     .message = nlohmann::json(status).dump(),
                                     .exchange_type = Config::Routes::exchangeType});
}

std::vector<UserId> NotificationOrchestrator::fetchChatMembers(long long chat_id) {
  return network_facade_->chats().getMembersOfChat(chat_id);
}

SocketNotifier::SocketNotifier(IUserSocketRepository *sock_manager) : socket_manager_(sock_manager) {}

bool SocketNotifier::notifyMember(long long user_id, nlohmann::json json_message,
                                       std::string type) {  // todo: implement enum
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

void RabbitNotificationPublisher::saveDeliveryStatus(const Message &msg,
                                             long long receiver_id) {  // todo: delete this
  LOG_INFO("saveDeliveryStatus message {} for receiver id {}", nlohmann::json(msg).dump(), receiver_id);
  MessageStatus status;
  status.message_id = msg.id;
  status.receiver_id = receiver_id;
  status.is_read = false;

  LOG_INFO("saveDeliveryStatus message_status {} for receiver id {}", nlohmann::json(status).dump(), receiver_id);
  saveMessageStatus(status);
}

std::optional<long long> NotificationOrchestrator::getChatIdOfMessage(long long message_id) {
  DBC_REQUIRE(message_id > 0);
  return network_facade_->messages().getChatIdOfMessage(message_id);
}

void RabbitNotificationPublisher::saveReaction(const Reaction &reaction) {
  mq_client_->publish(PublishRequest{.exchange = Config::Routes::exchange,
                                     .routing_key = Config::Routes::saveReaction,
                                     .message = nlohmann::json(reaction).dump(),
                                     .exchange_type = Config::Routes::exchangeType});
}

void RabbitNotificationPublisher::deleteReaction(const Reaction &reaction) {
  mq_client_->publish(PublishRequest{.exchange = Config::Routes::exchange,
                                     .routing_key = Config::Routes::deleteReaction,
                                     .message = nlohmann::json(reaction).dump(),
                                     .exchange_type = Config::Routes::exchangeType});
}
