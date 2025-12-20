#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include "dto/ChatBase.h"
#include "models/messagemodel.h"

using ChatId          = long long;
using ChatPtr         = std::shared_ptr<ChatBase>;
using MessageId       = long long;
using UserId          = long long;
using MessageModelPtr = std::shared_ptr<MessageModel>;
using ChatMap         = std::unordered_map<ChatId, ChatPtr>;
using MessageModelMap = std::unordered_map<MessageId, MessageModelPtr>;
using UserMap         = std::unordered_map<UserId, User>;
using OptionalUser    = std::optional<User>;

class DataManager {
 public:
  ChatPtr         getPrivateChatWithUser(UserId);
  MessageModelPtr getMessageModel(ChatId);
  ChatPtr         getChat(ChatId);
  int             getNumberOfExistingChats() const;
  void            clearAllChats();
  void            clearAllUsers();
  void            clearAllMessageModels();
  void            addChat(ChatPtr chat, MessageModelPtr message_model = nullptr);
  void            saveUser(User);
  void            clearAll();
  OptionalUser    getUser(UserId);

 protected:
  int             getNumberOfExistingModels() const;
  int             getNumberOfExistingUsers() const;

  ChatMap         chats_by_id_;
  UserMap         users_by_id_;
  MessageModelMap message_models_by_chat_id_;
};

#endif  // DATAMANAGER_H
