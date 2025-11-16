#include "messageservice/managers/MessageManager.h"

#include "GenericRepository.h"
#include "SqlExecutor.h"
#include "interfaces/BaseQuery.h"
#include "interfaces/ISqlExecutor.h"
#include "interfaces/ICacheService.h"

MessageManager::MessageManager(GenericRepository* repository, ISqlExecutor*  executor, ICacheService& cache)
    : repository_(repository), executor_(executor), cache_(cache) {}

bool MessageManager::saveMessage(Message& msg) {
  return repository_->save(msg);
}

std::optional<Message> MessageManager::getMessage(int message_id) {
  return repository_->findOne<Message>(message_id);
}

std::optional<MessageStatus> MessageManager::getMessageStatus(int message_id, int receiver_id) {
  auto        custom_query = QueryFactory::createSelect<MessageStatus>(executor_, cache_);
  custom_query->where("message_id", message_id).where("receiver_id", receiver_id).limit(1);

  auto res = custom_query->execute();
  LOG_INFO(
      "Found for message_id = '{}' and receiverId = '{}' status messages size "
      "= '{}'",
      message_id,
      receiver_id,
      (int)res.size());

  return res.empty() ? std::nullopt : std::make_optional(res.front());
}

std::vector<Message> MessageManager::getChatMessages(const GetMessagePack& pack) {
  auto        custom_query = QueryFactory::createSelect<Message>(executor_, cache_);
  custom_query->join("messages_status", "id = messages_status.message_id")
      .where("chat_id", pack.chat_id).limit(pack.limit)
      .where("messages_status.user_id", pack.user_id);

  custom_query->orderBy("timestamp", OrderDirection::DESC);

  if (pack.before_id > 0) {
    custom_query->where("id", "<", pack.before_id);
  }

  return custom_query->execute();
}

std::vector<MessageStatus> MessageManager::getMessagesStatus(const std::vector<Message>& messages, int receiver_id) {
  std::vector<MessageStatus> ans;

  for(const auto& msg: messages) {
    auto custom_query = QueryFactory::createSelect<MessageStatus>(executor_, cache_);
    custom_query->where("message_id", msg.id).where("receiver_id", receiver_id);
    auto returned_list = custom_query->execute();
    if(returned_list.size() != 1) LOG_WARN("Returned {}", returned_list.size());
    else ans.emplace_back(returned_list.front());
  }

  return ans;
}

[[nodiscard]] bool MessageManager::saveMessageStatus(MessageStatus& status) {
  return repository_->save(status);
}
