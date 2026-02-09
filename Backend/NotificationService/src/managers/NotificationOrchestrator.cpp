#include "notificationservice/managers/NotificationOrchestrator.h"

#include "Debug_profiling.h"
#include "NetworkFacade.h"
#include "config/Routes.h"
#include "interfaces/IRabitMQClient.h"
#include "notificationservice/IPublisher.h"
#include "notificationservice/ISubscriber.h"
#include "notificationservice/SocketNotifier.h"
#include "notificationservice/SocketRepository.h"
#include "utils.h"

NotificationOrchestrator::NotificationOrchestrator(INetworkFacade *network_facade, IPublisher *publisher,
                                                   INotifier *notifier)
    : network_facade_(network_facade), publisher_(publisher), notifier_(notifier) {}

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

void NotificationOrchestrator::onMessageDeleted(const std::string &payload) {
  auto parsed = utils::parsePayload<Message>(payload);
  if (!parsed) return;

  Message deleteted_message = *parsed;
  auto members_of_chat = fetchChatMembers(deleteted_message.chat_id);
  LOG_INFO("For chat id '{}' finded '{}' members", deleteted_message.chat_id, members_of_chat.size());
  LOG_INFO("Received deleted message {}", nlohmann::json(deleteted_message).dump());

  for (auto user_id : members_of_chat) {
    LOG_INFO("{} is member of chat {}", user_id, deleteted_message.chat_id);
    MessageStatus status;
    status.message_id = deleteted_message.id;
    status.receiver_id = user_id;

    publisher_->deleteMessageStatus(status);
    notifier_->notifyMember(user_id, deleteted_message, "delete_message");
  }
}

// void NotificationOrchestrator::onUserConnected(long long user_id, SocketPtr socket) {
//   socket_manager_->saveConnections(user_id, std::move(socket));
//   // notify users who communicate with this user
// }

void RabbitNotificationPublisher::saveMessage(const Message &message) {
  LOG_INFO("Send message from '{}': {}", message.sender_id, nlohmann::json(message).dump());

  // auto to_save = nlohmann::json(message);
  // to_save["event"] = "save_message";

  mq_client_->publish(PublishRequest{// TODO: Factory(?)
                                     .exchange = Config::Routes::exchange,
                                     .routing_key = Config::Routes::saveMessage,
                                     .message = nlohmann::json(message).dump(),
                                     .exchange_type = Config::Routes::exchangeType});
}

void RabbitNotificationPublisher::deleteMessageStatus(const MessageStatus &message_status) {
  mq_client_->publish(PublishRequest{// TODO: Factory(?)
                                     .exchange = Config::Routes::exchange,
                                     .routing_key = Config::Routes::deleteMessageStatus,
                                     .message = nlohmann::json(message_status).dump(),
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
    MessageStatus status;
    status.message_id = saved_message.id;
    status.receiver_id = user_id;
    status.is_read = false;

    publisher_->saveMessageStatus(status);
    notifier_->notifyMember(user_id, saved_message, "new_message");
  }
}

RabbitNotificationPublisher::RabbitNotificationPublisher(IEventPublisher *mq_client) : mq_client_(mq_client) {}

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
  if (json_message.is_null()) {
    return false;
  }

  auto socket = socket_manager_->getUserSocket(user_id);

  if (!socket) {
    LOG_INFO("User {} offline", user_id);
    return false;
  }

  if (type.empty()) {
    LOG_WARN("Type is empty");
  }

  utils::addFiledToJson(json_message, "type", type);
  socket->send_text(json_message.dump());

  return true;
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
