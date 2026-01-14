#ifndef MESSAGEUSECASE_H
#define MESSAGEUSECASE_H

#include <QObject>

class MessageModel;
class DataManager;
class MessageManager;
class TokenManager;
struct Message;
struct User;
struct ReactionInfo;

class MessageUseCase : public QObject {
  Q_OBJECT
 public:
  using MessageModelPtr = std::shared_ptr<MessageModel>;
  using Token = QString;

  MessageUseCase(DataManager *, std::unique_ptr<MessageManager> message_manager, TokenManager *token_manager);
  [[nodiscard]] MessageModel *getMessageModel(long long chat_id);
  [[nodiscard]] QList<Message> getChatMessages(long long chat_id, int limit = 20);
  void getChatMessagesAsync(long long chat_id);
  void addMessageToChat(Message &msg);
  void updateMessage(Message &msg);
  void deleteMessage(const Message &msg);
  void logout();
  void clearAllMessages();
  void saveReactionInfo(const std::vector<ReactionInfo>& reaction_infos);

 Q_SIGNALS:
  void messageAdded(const Message &);

 private:
  void onMessageReceived(const QString &message_text);
  void setupConnections();

  DataManager *data_manager_;
  std::unique_ptr<MessageManager> message_manager_;
  TokenManager *token_manager_;
};

#endif  // MESSAGEUSECASE_H
