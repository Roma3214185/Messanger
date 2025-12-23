#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <QObject>

#include "dto/ChatBase.h"
#include "models/messagemodel.h"
#include "dto/User.h"

class MessageModel;

using ChatId          = long long;
using ChatPtr         = std::shared_ptr<ChatBase>;
using MessageId       = long long;
using UserId          = long long;
using MessageModelPtr = std::shared_ptr<MessageModel>;
using ChatMap         = std::unordered_map<ChatId, ChatPtr>;
using MessageModelMap = std::unordered_map<MessageId, MessageModelPtr>;
using UserMap         = std::unordered_map<UserId, User>;
using OptionalUser    = std::optional<User>;
using ListMessage     = std::vector<Message>;

class DataManager : public QObject {
    Q_OBJECT
 public:
  ChatPtr         getPrivateChatWithUser(UserId);
  MessageModelPtr getMessageModel(ChatId);
  ChatPtr         getChat(ChatId);
  int             getNumberOfExistingChats() const;
  void            clearAllChats();
  void            clearAllUsers();
  void            clearAllMessageModels();
  void            addChat(ChatPtr chat, MessageModelPtr message_model = nullptr);
  void            saveUser(const User&);
  void            clearAll();
  OptionalUser    getUser(UserId);
  void            saveMessage(const Message& message);

Q_SIGNALS:
  void chatAdded(const ChatPtr& chat);
  void messageAdded(const Message& message);

 protected:
  int             getNumberOfExistingModels() const;
  int             getNumberOfExistingUsers() const;

  ChatMap         chats_by_id_;
  UserMap         users_by_id_;
  ListMessage     messages_;
  MessageModelMap message_models_by_chat_id_;

  std::mutex messages_mutex_;
  std::mutex chat_mutex_;
  std::mutex user_mutex_;
};

#endif  // DATAMANAGER_H
