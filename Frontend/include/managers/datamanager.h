#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <QObject>

#include "dto/ChatBase.h"
#include "dto/User.h"
#include "entities/Reaction.h"
#include "models/messagemodel.h"

class MessageModel;

using ChatId = long long;
using ChatPtr = std::shared_ptr<ChatBase>;
using MessageId = long long;
using UserId = long long;
using MessageModelPtr = std::shared_ptr<MessageModel>;
using ChatMap = std::unordered_map<ChatId, ChatPtr>;
using MessageModelMap = std::unordered_map<MessageId, MessageModelPtr>;
using UserMap = std::unordered_map<UserId, User>;
using OptionalUser = std::optional<User>;
using ListMessage = std::vector<Message>;
using ReactionsById = std::unordered_map<long long, ReactionInfo>;

class DataManager : public QObject {
  Q_OBJECT
 public:
  [[nodiscard]] ChatPtr getPrivateChatWithUser(UserId);
  [[nodiscard]] MessageModelPtr getMessageModel(ChatId);
  [[nodiscard]] ChatPtr getChat(ChatId);
  [[nodiscard]] int getNumberOfExistingChats() const noexcept;
  void clearAllChats();  // todo: think about clear chats authoatically clear
                         // message-models
  void clearAllUsers();
  void clearAllMessageModels();
  void addChat(const ChatPtr &chat, MessageModelPtr message_model = nullptr);

  void save(const User &user);
  void save(const Message &message);
  void save(const ReactionInfo &reaction_info);
  void save(const Reaction &reaction);

  template <typename T, template <typename...> class Container>
  void save(const Container<T> &entities) {
    for (const auto &entity : entities) {
      save(entity);
    }
  }

  void clearAll();
  [[nodiscard]] OptionalUser getUser(UserId);
  [[nodiscard]] std::optional<Message> getMessageById(const long long id);
  [[nodiscard]] int getNumberOfMessageModels() const noexcept {
    return static_cast<int>(message_models_by_chat_id_.size());
  }
  void deleteMessage(const Message &msg);
  void readMessage(long long message_id, long long readed_by);
  std::optional<ReactionInfo> getReactionInfo(long long reaction_id);
  void deleteReaction(const Reaction &reaction);
  std::vector<ReactionInfo> getEmojiesForMenu();

 Q_SIGNALS:
  void chatAdded(const ChatPtr &chat);
  void messageAdded(const Message &message);
  // add messageChanged or rename messageSaved(both added and changeed something)
  void messageDeleted(const Message &message);

 protected:
  [[nodiscard]] int getNumberOfExistingUsers() const noexcept;

 private:
  auto getIterMessageById(long long message_id);
  auto getIterMessageByLocalId(const QString &local_id);

  ChatMap chats_by_id_;
  UserMap users_by_id_;
  ListMessage messages_;
  MessageModelMap message_models_by_chat_id_;

  ReactionsById reactions_;

  std::mutex messages_mutex_;
  std::mutex chat_mutex_;
  std::mutex user_mutex_;
};

#endif  // DATAMANAGER_H
