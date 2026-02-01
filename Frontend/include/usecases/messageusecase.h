#ifndef MESSAGEUSECASE_H
#define MESSAGEUSECASE_H

#include <QObject>

class MessageModel;
class IMessageDataManager;
class MessageManager;
class TokenManager;
struct Message;
struct User;
struct ReactionInfo;

using MessageModelPtr = std::shared_ptr<MessageModel>;
using Token = QString;

class IMessageUseCase {
 public:
  virtual ~IMessageUseCase() = default;
  virtual MessageModel *getMessageModel(long long chat_id) = 0;
  virtual QList<Message> getChatMessages(long long chat_id, int limit = 20) = 0;
  virtual void getChatMessagesAsync(long long chat_id) = 0;
  virtual void deleteMessage(const Message &msg) = 0;
};

class MessageUseCase : public QObject, public IMessageUseCase {
  Q_OBJECT
 public:
  using MessageModelPtr = std::shared_ptr<MessageModel>;
  using Token = QString;

  MessageUseCase(IMessageDataManager *, std::unique_ptr<MessageManager> message_manager, TokenManager *token_manager);
  [[nodiscard]] MessageModel *getMessageModel(long long chat_id) override;
  [[nodiscard]] QList<Message> getChatMessages(long long chat_id, int limit = 20) override;
  void getChatMessagesAsync(long long chat_id) override;
  void deleteMessage(const Message &msg) override;

 Q_SIGNALS:
  void messageAdded(const Message &);

 private:
  IMessageDataManager *data_manager_;
  std::unique_ptr<MessageManager> message_manager_;
  TokenManager *token_manager_;
};

#endif  // MESSAGEUSECASE_H
