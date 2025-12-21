#ifndef MESSAGEUSECASE_H
#define MESSAGEUSECASE_H

#include <QObject>


class MessageModel;
class DataManager;
class MessageManager;
class TokenManager;
class Message;
class User;

class MessageUseCase : public QObject {
    Q_OBJECT
  public:
    using MessageModelPtr = std::shared_ptr<MessageModel>;
    using Token = QString;
    MessageUseCase(DataManager*, MessageManager* message_manager, TokenManager* token_manager);
    MessageModel* getMessageModel(long long chat_id);
    QList<Message> getChatMessages(long long chat_id, int limit = 20);
    void    fillChatHistory(long long chat_id);
    void    addMessageToChat(long long chat_id, const Message& msg);
    void    addOfflineMessageToChat(long long chat_id, const User&, const Message&);
    void    logout();
    void    clearAllMessages();

  Q_SIGNALS:
    void messageAdded(const Message&);

  private:
    void onMessageReceived(const QString& message_text);
    void setupConnections();
    void addMessageWithUpdatingChatList(const Message&, const User&, long long chat_id, MessageModelPtr);

    DataManager* data_manager_;
    MessageManager* message_manager_;
    TokenManager* token_manager_;
};

#endif // MESSAGEUSECASE_H

