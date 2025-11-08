#include "managers/MessageManager.h"
#include "Persistence/GenericRepository.h"
#include "Persistence/include/interfaces/BaseQuery.h"

MessageManager::MessageManager(GenericRepository* repository,
                               Batcher<Message>* message_batcher,
                               Batcher<MessageStatus>* messages_status_batcher)
    : repository_(repository),
      message_batcher_(message_batcher),
      messages_status_batcher_(messages_status_batcher) {}

bool MessageManager::saveMessage(Message& msg) {
  return repository_->save(msg);
}

std::optional<Message> MessageManager::getMessage(int message_id) {
  return repository_->findOne<Message>(message_id);
}

std::optional<MessageStatus> MessageManager::getMessageStatus(int message_id,
                                                              int receiver_id) {
  auto custom_query = QueryFactory::createSelect<MessageStatus>(repository_->getDatabase(), cache_);
  custom_query->where("id", message_id)
               .where("receiver_id", receiver_id)
               .limit(1);

  auto res = custom_query->execute();
  LOG_INFO(
      "Found for message_id = '{}' and receiverId = '{}' status messages size "
      "= '{}'",
      message_id, receiver_id, (int)res.size());

  return res.empty() ? std::nullopt : std::make_optional(res.front());
}

std::optional<int> MessageManager::getChatId(int message_id) {
  auto message = getMessage(message_id);
  if (!message) return std::nullopt;
  return message->chat_id;
}

std::vector<Message> MessageManager::getChatMessages(int chat_id, int limit,
                                                     int before_id) {
  auto custom_query = QueryFactory::createSelect<Message>(repository_->getDatabase(), cache_);
  custom_query->where("chat_id", chat_id).limit(limit);
  custom_query->orderBy("timestamp", "DESC");

  if (before_id > 0) {
    custom_query->where("id", "<", before_id);
  }

  return custom_query->execute();
}

[[nodiscard]] bool MessageManager::saveMessageStatus(MessageStatus& status) {
  return repository_->save(status);
}

std::vector<MessageStatus> MessageManager::getUndeliveredMessages(int user_id) {
  auto custom_query = QueryFactory::createSelect<MessageStatus>(repository_->getDatabase(), cache_);
  custom_query->where("receiver_id", user_id)
                .where("is_read", "0");
  return custom_query->execute();
}
