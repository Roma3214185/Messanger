#ifndef CHATUSECASE_H
#define CHATUSECASE_H

#include <QObject>

#include "dto/ChatBase.h"
#include "managers/chatmanager.h"

class ChatModel;
class TokenManager;
class IChatDataManager;

using Token = QString;
using ChatId = long long;
using UserId = long long;
using ChatPtr = std::shared_ptr<ChatBase>;

class ChatUseCase : public QObject {
  Q_OBJECT
 public:
  using Token = QString;
  using ChatId = long long;
  using UserId = long long;
  using ChatPtr = std::shared_ptr<ChatBase>;

  ChatUseCase(std::unique_ptr<ChatManager>, IChatDataManager *, TokenManager *);
  [[nodiscard]] ChatPtr loadChat(ChatId);
  QList<ChatPtr> loadChats();
  void loadChatsAsync();
  ChatPtr createPrivateChat(UserId);
  ChatPtr getPrivateChatWithUser(UserId);

  void createChat(ChatId);
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
