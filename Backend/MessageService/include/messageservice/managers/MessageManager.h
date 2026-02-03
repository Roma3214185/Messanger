#ifndef BACKEND_MESSAGE_SERVICE_MESSAGEMANAGER_H
#define BACKEND_MESSAGE_SERVICE_MESSAGEMANAGER_H

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

class IMessageManager {
 public:
  virtual ~IMessageManager() = default;
  virtual bool saveMessage(Message &message) = 0;
  virtual std::optional<Message> getMessage(long long message_id) = 0;
  virtual std::optional<MessageStatus> getMessageStatus(long long message_id, long long receiver_id) = 0;
  virtual std::vector<Message> getChatMessages(const GetMessagePack &) = 0;
  virtual bool saveMessageStatus(MessageStatus &status) = 0;
  // virtual std::vector<MessageStatus> getUndeliveredMessages(long long user_id) = 0;
  virtual std::vector<MessageStatus> getMessagesStatus(const std::vector<Message> &messages, long long receiver_id) = 0;
  virtual bool updateMessage(const Message &message) = 0;
  virtual bool deleteMessage(const Message &message) = 0;
  virtual std::vector<MessageStatus> getReadedMessageStatuses(long long message_id) = 0;
  virtual bool saveMessageReaction(const Reaction &reaction) = 0;
  virtual bool deleteMessageReaction(const Reaction &reaction) = 0;
  virtual bool saveMessageReactionInfo(const std::vector<ReactionInfo> &reaction_infos) = 0;
  virtual std::pair<std::unordered_map<ReactionInfo, int>, std::optional<int>> getReactions(long long message_id,
                                                                                            long long receiver_id) = 0;
  virtual std::optional<ReactionInfo> getReactionInfo(long long message_reaction_id) = 0;
};

class MessageManager : public IMessageManager {
 public:
  MessageManager(GenericRepository *rep, IIdGenerator *generator);
  [[nodiscard]] bool saveMessage(Message &message) override;
  std::optional<Message> getMessage(long long message_id) override;
  std::optional<MessageStatus> getMessageStatus(long long message_id, long long receiver_id) override;
  virtual std::vector<Message> getChatMessages(const GetMessagePack &) override;
  [[nodiscard]] bool saveMessageStatus(MessageStatus &status) override;
  // std::vector<MessageStatus> getUndeliveredMessages(long long user_id) override;
  std::vector<MessageStatus> getMessagesStatus(const std::vector<Message> &messages, long long receiver_id) override;
  [[nodiscard]] bool updateMessage(const Message &message) override;
  [[nodiscard]] bool deleteMessage(const Message &message) override;
  std::vector<MessageStatus> getReadedMessageStatuses(long long message_id) override;
  bool saveMessageReaction(const Reaction &reaction) override;
  bool deleteMessageReaction(const Reaction &reaction) override;
  bool saveMessageReactionInfo(const std::vector<ReactionInfo> &reaction_infos) override;

  std::pair<std::unordered_map<ReactionInfo, int>, std::optional<int>> getReactions(long long message_id,
                                                                                    long long receiver_id) override;
  std::optional<ReactionInfo> getReactionInfo(long long message_reaction_id) override;

 private:
  GenericRepository *repository_;
  IIdGenerator *generator_;
};

#endif  // BACKEND_MESSAGE_SERVICE_MESSAGEMANAGER_H
