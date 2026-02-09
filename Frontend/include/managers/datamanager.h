#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <QObject>

#include "dto/ChatBase.h"
#include "dto/User.h"
#include "entities/Reaction.h"
#include "models/messagemodel.h"

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

class MessageStatus;

class IReactionDataManager {
 public:
  virtual ~IReactionDataManager() = default;

  virtual void save(const ReactionInfo &reaction) = 0;
  virtual void save(const Reaction &reaction) = 0;
  virtual void deleteReaction(const Reaction &reaction) = 0;
  virtual std::optional<ReactionInfo> getReactionInfo(long long id) = 0;
  virtual std::vector<ReactionInfo> getEmojiesForMenu() = 0;
};

class IMessageStatusDataManager {
 public:
  virtual ~IMessageStatusDataManager() = default;
  virtual void save(const MessageStatus &message_status) = 0;
};

class IMessageDataManager {
 public:
  virtual ~IMessageDataManager() = default;

  virtual MessageModelPtr getMessageModel(ChatId) = 0;
  virtual std::optional<Message> getMessageById(long long id) = 0;

  virtual void save(const Message &message) = 0;
  virtual void deleteMessage(const Message &msg) = 0;

  virtual int getNumberOfMessageModels() const noexcept = 0;
  virtual void clearAllMessageModels() = 0;
};

class IUserDataManager {
 public:
  virtual ~IUserDataManager() = default;

  virtual OptionalUser getUser(UserId) = 0;
  virtual void save(const User &user) = 0;
  virtual void clearAllUsers() = 0;

 protected:
  virtual int getNumberOfExistingUsers() const noexcept = 0;
};

class IChatDataManager {
 public:
  virtual ~IChatDataManager() = default;

  virtual ChatPtr getPrivateChatWithUser(UserId) = 0;
  virtual ChatPtr getChat(ChatId) = 0;
  virtual int getNumberOfExistingChats() const noexcept = 0;

  virtual void save(const ChatPtr &chat, MessageModelPtr message_model = nullptr) = 0;

  virtual void clearAllChats() = 0;
};

class DataManager : public QObject,
                    public IChatDataManager,
                    public IUserDataManager,
                    public IMessageDataManager,
                    public IReactionDataManager,
                    public IMessageStatusDataManager {
  Q_OBJECT
 public:
  [[nodiscard]] ChatPtr getPrivateChatWithUser(UserId) override;
  [[nodiscard]] MessageModelPtr getMessageModel(ChatId) override;
  [[nodiscard]] ChatPtr getChat(ChatId) override;
  [[nodiscard]] int getNumberOfExistingChats() const noexcept override;
  void clearAllChats() override;
  void clearAllUsers() override;
  void clearAllMessageModels() override;
  void save(const ChatPtr &chat, MessageModelPtr message_model = nullptr) override;

  void save(const User &user) override;
  void save(const Message &message) override;
  void save(const ReactionInfo &reaction_info) override;
  void save(const Reaction &reaction) override;
  void save(const MessageStatus &) override;

  template <typename T, template <typename...> class Container>
  void save(const Container<T> &entities) {
    for (const auto &entity : entities) {
      save(entity);
    }
  }

  void clearAll();
  [[nodiscard]] OptionalUser getUser(UserId) override;
  [[nodiscard]] std::optional<Message> getMessageById(const long long id) override;
  [[nodiscard]] int getNumberOfMessageModels() const noexcept override {
    return static_cast<int>(message_models_by_chat_id_.size());
  }

  void deleteMessage(const Message &msg) override;
  std::optional<ReactionInfo> getReactionInfo(long long reaction_id) override;
  void deleteReaction(const Reaction &reaction) override;
  std::vector<ReactionInfo> getEmojiesForMenu() override;

 Q_SIGNALS:
  void chatAdded(const ChatPtr &chat);
  void messageAdded(const Message &message);
  // add messageChanged or rename messageSaved(both added and changeed something)
  void messageDeleted(const Message &message);

 protected:
  [[nodiscard]] int getNumberOfExistingUsers() const noexcept override;

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
