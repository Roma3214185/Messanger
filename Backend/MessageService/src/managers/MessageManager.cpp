#include "messageservice/managers/MessageManager.h"

#include "GenericRepository.h"
#include "interfaces/ICacheService.h"
#include "interfaces/IIdGenerator.h"
#include "interfaces/ISqlExecutor.h"
#include "messageservice/dto/GetMessagePack.h"

MessageManager::MessageManager(GenericRepository *repository, ISqlExecutor *executor, IIdGenerator *generator,
                               ICacheService &cache)
    : repository_(repository), executor_(executor), generator_(generator), cache_(cache) {}

bool MessageManager::saveMessage(Message &msg) {
  msg.id = generator_->generateId();
  return repository_->save(msg);
}

std::optional<Message> MessageManager::getMessage(long long message_id) {
  return repository_->findOne<Message>(message_id);
}

std::optional<MessageStatus> MessageManager::getMessageStatus(long long message_id, long long receiver_id) {
  auto custom_query = QueryFactory::createSelect<MessageStatus>(executor_, cache_);
  custom_query->where(MessageStatusTable::MessageId, message_id)
      .where(MessageStatusTable::ReceiverId, receiver_id)
      .limit(1);
  auto res = custom_query->execute();
  auto select_res = QueryFactory::getSelectResult(res);
  return select_res.result.empty() ? std::nullopt : std::make_optional(select_res.result.front());
}

std::vector<Message> MessageManager::getChatMessages(const GetMessagePack &pack) {
  PROFILE_SCOPE();
  LOG_INFO("Start MessageManager::getChatMessages");
  auto custom_query = QueryFactory::createSelect<Message>(executor_, cache_);
  custom_query
      ->join(MessageStatusTable::Table, MessageTable::Id, MessageStatusTable::fullField(MessageStatusTable::MessageId))
      .where(MessageTable::ChatId, pack.chat_id)
      .limit(pack.limit)
      .where(MessageStatusTable::fullField(MessageStatusTable::ReceiverId), pack.user_id);

  custom_query->orderBy(MessageTable::Timestamp, OrderDirection::DESC);

  if (pack.before_id > 0) {
    custom_query->where(MessageTable::Id, Operator::Less, pack.before_id);
  }
  LOG_INFO("Query to select fully created");
  auto res = custom_query->execute();
  LOG_INFO("Query executed");
  return QueryFactory::getSelectResult(res).result;
}

std::vector<MessageStatus> MessageManager::getMessagesStatus(const std::vector<Message> &messages,
                                                             long long receiver_id) {
  std::vector<MessageStatus> ans;

  for (const auto &msg : messages) {
    auto custom_query = QueryFactory::createSelect<MessageStatus>(executor_, cache_);
    custom_query->where(MessageStatusTable::MessageId, msg.id).where(MessageStatusTable::ReceiverId, receiver_id);
    auto res = custom_query->execute();
    auto returned_list = QueryFactory::getSelectResult(res).result;
    if (returned_list.size() != 1) {
      LOG_WARN("Returned {}", returned_list.size());
    } else {
      ans.emplace_back(returned_list.front());
    }
  }

  return ans;
}

bool MessageManager::saveMessageStatus(MessageStatus &status) { return repository_->save(status); }

bool MessageManager::updateMessage(const Message &message) { return repository_->save(message); }

bool MessageManager::deleteMessage(const Message &message) {
  if (!repository_->deleteEntity<Message>(message)) {
    LOG_INFO("Delete entity failed");
    return false;
  }

  auto query = QueryFactory::createDelete<MessageStatus>(executor_, cache_);
  query->where(MessageStatusTable::MessageId, message.id);
  auto res = query->execute();
  return QueryFactory::getDeleteResult(res).success;
}

std::vector<MessageStatus> MessageManager::getReadedMessageStatuses(long long message_id) {
  DBC_REQUIRE(message_id > 0);
  auto query = QueryFactory::createSelect<MessageStatus>(executor_, cache_);
  query->where(MessageStatusTable::MessageId, message_id).where(MessageStatusTable::IsRead, 1);
  auto res = query->execute();
  return QueryFactory::getSelectResult(res).result;
}

bool MessageManager::saveMessageReaction(const Reaction &reaction) {
  DBC_REQUIRE(reaction.checkInvariants());
  return repository_->save(reaction);
}

bool MessageManager::deleteMessageReaction(const Reaction &reaction) {
  DBC_REQUIRE(reaction.checkInvariants());
  auto query = QueryFactory::createDelete<Reaction>(executor_, cache_);
  query->where(MessageReactionTable::MessageId, reaction.message_id);
  query->where(MessageReactionTable::ReceiverId, reaction.receiver_id);
  // todo: in future premium users can have a couple of reactions, so query->where(MessageReactionTable::ReactionId,
  // reaction.reaction_id);
  auto res = query->execute();
  return QueryFactory::getDeleteResult(res).success;
}

std::pair<std::unordered_map<ReactionInfo, int>, std::optional<int>> MessageManager::getReactions(
    long long message_id, long long receiver_id) {
  auto custom_query = QueryFactory::createSelect<Reaction>(executor_, cache_);
  custom_query->where(MessageReactionTable::MessageId, message_id);
  auto res = custom_query->execute();
  std::vector<Reaction> vector_of_reactions = QueryFactory::getSelectResult(res).result;

  std::optional<long long> receiver_id_reactions;
  std::unordered_map<long long, int> message_reactions;

  for (const Reaction &reaction : vector_of_reactions) {
    message_reactions[reaction.reaction_id]++;
    if (reaction.receiver_id == receiver_id) receiver_id_reactions = reaction.reaction_id;
  }

  std::unordered_map<ReactionInfo, int> message_reactions_infos;
  for (const auto &[reaction_id, reaction_cnt] : message_reactions) {
    std::optional<ReactionInfo> reaction_info = getReactionInfo(reaction_id);
    if (reaction_info.has_value()) {
      message_reactions_infos.emplace(reaction_info.value(), reaction_cnt);
    } else {
      LOG_ERROR("Not found reaction with id {}", reaction_id);
    }
  }

  return std::make_pair(message_reactions_infos, receiver_id_reactions);
}

std::optional<ReactionInfo> MessageManager::getReactionInfo(long long message_reaction_id) {
  DBC_REQUIRE(message_reaction_id > 0);
  auto custom_query = QueryFactory::createSelect<ReactionInfo>(executor_, cache_);
  custom_query->where(MessageReactionInfoTable::Id, message_reaction_id);
  auto res = custom_query->execute();
  std::vector<ReactionInfo> vector_of_reactions_infos = QueryFactory::getSelectResult(res).result;
  return vector_of_reactions_infos.empty() ? std::nullopt : std::make_optional(vector_of_reactions_infos.front());
}

bool MessageManager::saveMessageReactionInfo(const std::vector<ReactionInfo> &reaction_infos) {
  for (auto &reaction_info : reaction_infos) {
    if (!repository_->save(reaction_info)) return false;  // todo: make pipeline
  }
  return true;
}
