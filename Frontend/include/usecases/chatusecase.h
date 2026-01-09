#ifndef CHATUSECASE_H
#define CHATUSECASE_H

#include <QObject>

#include "dto/ChatBase.h"

class ChatManager;
class ChatModel;
class TokenManager;
class DataManager;

class ChatUseCase : public QObject {
  Q_OBJECT
 public:
  using Token = QString;
  using ChatId = long long;
  using UserId = long long;
  using ChatPtr = std::shared_ptr<ChatBase>;

  ChatUseCase(std::unique_ptr<ChatManager>, DataManager *, ChatModel *, TokenManager *);
  [[nodiscard]] ChatPtr loadChat(ChatId);
  QList<ChatPtr> loadChats();
  void loadChatsAsync();
  ChatPtr createPrivateChat(UserId);
  ChatPtr getPrivateChatWithUser(UserId);

  void addChat(const ChatPtr &);
  void createChat(ChatId);
  int getNumberOfExistingChats() const;

  [[nodiscard]] ChatPtr getChat(ChatId);
  void clearAllChats();
  QModelIndex indexByChatId(ChatId);

  void logout();

 private:
  std::unique_ptr<ChatManager> chat_manager_;
  DataManager *data_manager_;
  ChatModel *chat_model_;
  TokenManager *token_manager_;
};

#endif  // CHATUSECASE_H
