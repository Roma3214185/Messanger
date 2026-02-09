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

class IMessageCommandService {
public:
    virtual ~IMessageCommandService() = default;

    virtual bool saveMessage(Message &message) = 0;
    virtual bool saveMessageStatus(MessageStatus &status) = 0;
    virtual bool updateMessage(const Message &message) = 0;
    virtual bool deleteMessage(const Message &message) = 0;

    virtual bool saveMessageReaction(const Reaction &reaction) = 0;
    virtual bool deleteMessageReaction(const Reaction &reaction) = 0;
    virtual bool saveMessageReactionInfo(const std::vector<ReactionInfo> &reaction_infos) = 0;
};

class IMessageQueryService {
public:
    virtual ~IMessageQueryService() = default;

    virtual std::optional<Message> getMessage(long long message_id) = 0;
    virtual std::optional<MessageStatus> getMessageStatus(long long message_id, long long receiver_id) = 0;
    virtual std::vector<Message> getChatMessages(const GetMessagePack &pack) = 0;
    virtual std::vector<MessageStatus> getMessagesStatus(const std::vector<Message> &messages, long long receiver_id) = 0;

    virtual std::vector<MessageStatus> getReadedMessageStatuses(long long message_id) = 0;
    virtual std::pair<std::unordered_map<ReactionInfo, int>, std::optional<int>> getReactions(long long message_id,
                                                                                              long long receiver_id) = 0;
    virtual std::optional<ReactionInfo> getReactionInfo(long long message_reaction_id) = 0;
};

class MessageCommandManager : public IMessageCommandService {
 public:
  MessageCommandManager(GenericRepository *rep, IIdGenerator *generator);
  bool saveMessage(Message &message) override;
  bool saveMessageStatus(MessageStatus &status) override;
  bool updateMessage(const Message &message) override;
  bool deleteMessage(const Message &message) override;

  //todo: make new microservice
  bool saveMessageReaction(const Reaction &reaction) override;
  bool deleteMessageReaction(const Reaction &reaction) override;
  bool saveMessageReactionInfo(const std::vector<ReactionInfo> &reaction_infos) override;

  private:
    GenericRepository *repository_;
    IIdGenerator *generator_;
};

class MessageQueryManager : public IMessageQueryService {
public:
    MessageQueryManager(ISqlExecutor *executor, ICacheService &cache);
    std::vector<Message> getChatMessages(const GetMessagePack &pack) override;
    std::optional<Message> getMessage(long long message_id) override;
    std::vector<MessageStatus> getMessagesStatus(const std::vector<Message> &messages, long long receiver_id) override;
    std::optional<MessageStatus> getMessageStatus(long long message_id, long long receiver_id) override;

    //temp
    std::vector<MessageStatus> getReadedMessageStatuses(long long message_id) override;
    std::pair<std::unordered_map<ReactionInfo, int>, std::optional<int>> getReactions(long long message_id,
                                                                                      long long receiver_id) override;
    std::optional<ReactionInfo> getReactionInfo(long long message_reaction_id) override;

private:
    ISqlExecutor *executor_;
    ICacheService &cache_;
};

#endif  // BACKEND_MESSAGE_SERVICE_MESSAGEMANAGER_H
