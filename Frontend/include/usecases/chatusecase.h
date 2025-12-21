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

    ChatUseCase(ChatManager*, DataManager*, ChatModel*, TokenManager*);
    ChatPtr loadChat(ChatId);
    QList<ChatPtr> loadChat();
    ChatPtr createPrivateChat(UserId);
    ChatPtr getPrivateChatWithUser(UserId);

    void addChat(const ChatPtr&);
    void addChatInFront(const ChatPtr&);
    void createChat(ChatId);
    int getNumberOfExistingChats() const;

    ChatPtr getChat(ChatId);
    void clearAllChats();
    QModelIndex indexByChatId(ChatId);

    void logout();

  Q_SIGNALS:
    void chatAdded(ChatId);
    void chatLoaded(ChatId);

  private:
    ChatManager* chat_manager_;
    DataManager* data_manager_;
    ChatModel* chat_model_;
    TokenManager* token_manager_;
};

#endif // CHATUSECASE_H
