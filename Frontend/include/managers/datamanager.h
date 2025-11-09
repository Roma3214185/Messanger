#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include "dto/ChatBase.h"
#include "models/messagemodel.h"

using ChatId          = int;
using ChatPtr         = std::shared_ptr<ChatBase>;
using MessageModelPtr = std::shared_ptr<MessageModel>;
using ChatMap         = std::unordered_map<ChatId, ChatPtr>;
using MessageModelMap = std::unordered_map<ChatId, MessageModelPtr>;

class DataManager {
 public:
  ChatPtr         getPrivateChatWithUser(int user_id);
  MessageModelPtr getMessageModel(int chat_id);
  void            addMessageModel(int chat_id, MessageModelPtr message_model);
  ChatPtr         getChat(int chat_id);
  int             getNumberOfExistingChats() const;
  void            clearAllChats();
  void            clearAllMessageModels();
  void            addChat(ChatPtr chat);

 private:
  ChatMap         chats_by_id_;
  MessageModelMap message_models_by_chat_id_;
};

#endif  // DATAMANAGER_H
