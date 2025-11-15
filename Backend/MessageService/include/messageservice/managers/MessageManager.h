#ifndef BACKEND_MESSAGE_SERVICE_MESSAGEMANAGER_H
#define BACKEND_MESSAGE_SERVICE_MESSAGEMANAGER_H

#include "Batcher.h"
#include "GenericRepository.h"
#include "RedisCache.h"
#include "entities/Message.h"
#include "entities/MessageStatus.h"

class GenericRepository;
class ISqlExecutor;

struct GetMessagePack {
    int chat_id;
    int limit;
    int before_id;
    int user_id;
};

class MessageManager {
 public:
  MessageManager(GenericRepository* rep, ISqlExecutor*  executor);
  [[nodiscard]] bool           saveMessage(Message& message);
  std::optional<Message>       getMessage(int message_id);
  std::optional<MessageStatus> getMessageStatus(int message_id, int receiver_id);
  std::optional<int>           getChatId(int message_id);
  virtual std::vector<Message> getChatMessages(const GetMessagePack&);
  [[nodiscard]] bool           saveMessageStatus(MessageStatus& status);
  std::vector<MessageStatus>   getUndeliveredMessages(int user_id);
  std::vector<MessageStatus>   getMessagesStatus(const std::vector<Message>& messages, int receiver_id);

 private:
  RedisCache&             cache_ = RedisCache::instance();
  GenericRepository*      repository_;
  ISqlExecutor*           executor_;
};

#endif  // BACKEND_MESSAGE_SERVICE_MESSAGEMANAGER_H
