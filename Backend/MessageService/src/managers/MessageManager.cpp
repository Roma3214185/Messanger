#include "managers/MessageManager.h"
#include "Persistence/GenericRepository.h"

MessageManager::MessageManager(GenericRepository* repository,
                               Batcher<Message>* message_batcher,
                               Batcher<MessageStatus>* messages_status_batcher)
    : repository_(repository),
      message_batcher_(message_batcher),
      messages_status_batcher_(messages_status_batcher) {}

bool MessageManager::saveMessage(Message& msg) {
  //message_batcher_->save(msg);
  return repository_->save(msg);
}

std::optional<Message> MessageManager::getMessage(int message_id) {
  return repository_->findOne<Message>(message_id);
}

std::optional<MessageStatus> MessageManager::getMessageStatus(int message_id,
                                                              int receiver_id) {
  auto custom_query = repository_->query<MessageStatus>()
               .filter("id", message_id)
               .filter("receiver_id", receiver_id)
               .limit(1);

  auto res = custom_query.execute();
  LOG_INFO(
      "Found for message_id = '{}' and receiverId = '{}' status messages size "
      "= '{}'",
      message_id, receiver_id, (int)res.size());

  if (res.empty()) return std::nullopt;
  return res.front();
}

std::optional<int> MessageManager::getChatId(int message_id) {
  auto message = getMessage(message_id);
  if (!message) return std::nullopt;
  return message->chat_id;
}

std::vector<Message> MessageManager::getChatMessages(int chat_id, int limit,
                                                     int before_id) {
  auto custom_query = repository_->query<Message>()
               .filter("chat_id", chat_id)
               .orderBy("timestamp", "DESC")
               .limit(limit);

  if (before_id > 0) {
    custom_query.filter("id", "<", before_id);
  }

  return custom_query.execute();
}

[[nodiscard]] bool MessageManager::saveMessageStatus(MessageStatus& status) {
  return repository_->save(status);
}

std::vector<MessageStatus> MessageManager::getUndeliveredMessages(int user_id) {
  auto custom_query = repository_->query<MessageStatus>()
                 .filter("receiver_id", user_id)
                            .filter("is_read", "0");
  auto res = custom_query.execute();
  LOG_INFO("getUndeliveredMessages return '{}' messages for userId '{}'",
           res.size(), user_id);
  return res;
}
