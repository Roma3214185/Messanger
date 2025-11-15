#ifndef MESSAGEMANAGER_H
#define MESSAGEMANAGER_H

#include "Batcher.h"
#include "GenericRepository.h"
#include "RedisCache.h"
#include "entities/Message.h"
#include "entities/MessageStatus.h"

class GenericRepository;

class MessageManager {
 public:
  MessageManager(GenericRepository* rep);
  [[nodiscard]] bool           saveMessage(Message& message);
  std::optional<Message>       getMessage(int message_id);
  std::optional<MessageStatus> getMessageStatus(int message_id, int receiver_id);
  std::optional<int>           getChatId(int message_id);
  std::vector<Message>         getChatMessages(int chat_id, int limit, int before_id);
  [[nodiscard]] bool           saveMessageStatus(MessageStatus& status);
  std::vector<MessageStatus>   getUndeliveredMessages(int user_id);

 private:
  RedisCache&             cache_ = RedisCache::instance();
  GenericRepository*      repository_;
};

#endif  // MESSAGEMANAGER_H
