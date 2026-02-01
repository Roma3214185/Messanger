#ifndef CHATUSECASE_H
#define CHATUSECASE_H

#include <QObject>

#include "dto/ChatBase.h"

class ChatManager;
class ChatModel;
class TokenManager;
class IChatDataManager;

using Token = QString;
using ChatId = long long;
using UserId = long long;
using ChatPtr = std::shared_ptr<ChatBase>;

class IChatUseCase {
 public:
  virtual ~IChatUseCase() = default;
  virtual ChatPtr loadChat(ChatId) = 0;
  virtual QList<ChatPtr> loadChats() = 0;
  virtual void loadChatsAsync() = 0;
  virtual ChatPtr getPrivateChatWithUser(UserId) = 0;
  virtual void createChat(ChatId) = 0;
  // virtual QModelIndex indexByChatId(ChatId)= 0;

 private:
  virtual ChatPtr createPrivateChat(UserId) = 0;
};

class ChatUseCase : public QObject, public IChatUseCase {
  Q_OBJECT
 public:
  using Token = QString;
  using ChatId = long long;
  using UserId = long long;
  using ChatPtr = std::shared_ptr<ChatBase>;

  ChatUseCase(std::unique_ptr<ChatManager>, IChatDataManager *, TokenManager *);
  [[nodiscard]] ChatPtr loadChat(ChatId) override;
  QList<ChatPtr> loadChats() override;
  void loadChatsAsync() override;
  ChatPtr createPrivateChat(UserId) override;
  ChatPtr getPrivateChatWithUser(UserId) override;

  void createChat(ChatId) override;
  int getNumberOfExistingChats() const;

  [[nodiscard]] ChatPtr getChat(ChatId);
  void clearAllChats();

  void logout();

 private:
  std::unique_ptr<ChatManager> chat_manager_;
  IChatDataManager *data_manager_;
  TokenManager *token_manager_;
};

#endif  // CHATUSECASE_H
