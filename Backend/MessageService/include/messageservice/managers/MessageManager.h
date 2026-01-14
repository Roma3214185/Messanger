#ifndef BACKEND_MESSAGE_SERVICE_MESSAGEMANAGER_H
#define BACKEND_MESSAGE_SERVICE_MESSAGEMANAGER_H

#include "Batcher.h"
#include "GenericRepository.h"
#include "RedisCache.h"
#include "entities/Message.h"
#include "entities/MessageStatus.h"
#include "entities/Reaction.h"
#include "entities/ReactionInfo.h"

class GenericRepository;
class ISqlExecutor;
class ICacheService;
class GetMessagePack;
class IIdGenerator;

class MessageManager {
 public:
  MessageManager(GenericRepository *rep, ISqlExecutor *executor, IIdGenerator *generator,
                 ICacheService &cache = RedisCache::instance());
  [[nodiscard]] bool saveMessage(Message &message);
  std::optional<Message> getMessage(long long message_id);
  std::optional<MessageStatus> getMessageStatus(long long message_id, long long receiver_id);
  virtual std::vector<Message> getChatMessages(const GetMessagePack &);
  [[nodiscard]] bool saveMessageStatus(MessageStatus &status);
  std::vector<MessageStatus> getUndeliveredMessages(long long user_id);
  std::vector<MessageStatus> getMessagesStatus(const std::vector<Message> &messages, long long receiver_id);
  [[nodiscard]] bool updateMessage(const Message &message);
  [[nodiscard]] bool deleteMessage(const Message &message);
  std::vector<MessageStatus> getReadedMessageStatuses(long long message_id);
  bool saveMessageReaction(const Reaction &reaction);
  bool deleteMessageReaction(const Reaction &reaction);
  bool saveMessageReactionInfo(const std::vector<ReactionInfo>& reaction_infos);

  std::pair<std::unordered_map<ReactionInfo, int>, std::optional<int>> getReactions(long long message_id, long long receiver_id);
  std::optional<ReactionInfo> getReactionInfo(long long message_reaction_id);

  GenericRepository *repository_;
  ISqlExecutor *executor_;
  IIdGenerator *generator_;
  ICacheService &cache_;
};

#endif  // BACKEND_MESSAGE_SERVICE_MESSAGEMANAGER_H
