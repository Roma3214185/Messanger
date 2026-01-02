#include "messageservice/managers/MessageManager.h"

#include "GenericRepository.h"
#include "interfaces/BaseQuery.h"
#include "interfaces/ISqlExecutor.h"
#include "interfaces/ICacheService.h"
#include "messageservice/dto/GetMessagePack.h"
#include "interfaces/IIdGenerator.h"

MessageManager::MessageManager(GenericRepository* repository, ISqlExecutor*  executor, IIdGenerator* generator, ICacheService& cache)
    : repository_(repository), executor_(executor), generator_(generator), cache_(cache) {}

bool MessageManager::saveMessage(Message& msg) {
  msg.id = generator_->generateId();
  return repository_->save(msg);
}

std::optional<Message> MessageManager::getMessage(long long message_id) {
  return repository_->findOne<Message>(message_id);
}

std::optional<MessageStatus> MessageManager::getMessageStatus(long long message_id, long long receiver_id) {
  auto        custom_query = QueryFactory::createSelect<MessageStatus>(executor_, cache_);
  custom_query
      ->where(MessageStatusTable::MessageId, message_id)
      .where(MessageStatusTable::ReceiverId, receiver_id).limit(1);
  auto res = custom_query->execute();
  return res.empty() ? std::nullopt : std::make_optional(res.front());
}

std::vector<Message> MessageManager::getChatMessages(const GetMessagePack& pack) {
  auto        custom_query = QueryFactory::createSelect<Message>(executor_, cache_);
  custom_query
      ->join(MessageStatusTable::Table, MessageTable::Id, MessageStatusTable::fullField(
                                                              MessageStatusTable::MessageId))
      .where(MessageTable::ChatId, pack.chat_id).limit(pack.limit)
      .where(MessageStatusTable::fullField(MessageStatusTable::ReceiverId), pack.user_id);

  custom_query->orderBy(MessageTable::Timestamp, OrderDirection::DESC);

  if (pack.before_id > 0) {
    custom_query->where(MessageTable::Id, Operator::Less, pack.before_id);
  }

  return custom_query->execute();
}

std::vector<MessageStatus> MessageManager::getMessagesStatus(const std::vector<Message>& messages, long long receiver_id) {
  std::vector<MessageStatus> ans;

  for(const auto& msg: messages) {
    auto custom_query = QueryFactory::createSelect<MessageStatus>(executor_, cache_);
    custom_query->where(MessageStatusTable::MessageId, msg.id).where(MessageStatusTable::ReceiverId, receiver_id);
    auto returned_list = custom_query->execute();
    if(returned_list.size() != 1) LOG_WARN("Returned {}", returned_list.size());
    else ans.emplace_back(returned_list.front());
  }

  return ans;
}

bool MessageManager::saveMessageStatus(MessageStatus& status) {
  return repository_->save(status);
}

bool MessageManager::updateMessage(const Message& message) {
  return repository_->save(message);
}

bool MessageManager::deleteMessage(const Message& message) {
  if(!repository_->deleteEntity<Message>(message)) {
    LOG_INFO("Delete entity failed");
    return false;
  }

  auto query = QueryFactory::createDelete<MessageStatus>(executor_, cache_);
  query->where(MessageStatusTable::MessageId, message.id);
  auto res = query->execute(); //todo: check res for
  return true;
}

std::vector<MessageStatus> MessageManager::getReadedMessageStatuses(long long message_id) {
  DBC_REQUIRE(message_id > 0);
  auto query = QueryFactory::createSelect<MessageStatus>(executor_, cache_);
  query->where(MessageStatusTable::MessageId, message_id)
      .where(MessageStatusTable::IsRead, 1);
  auto res = query->execute();
  LOG_INFO("getReadedMessageStatuses for id {} is size {} MessageStatus", message_id, res.size());
  return res;
}
