#ifndef MESSAGEUSECASE_H
#define MESSAGEUSECASE_H

#include <QObject>

#include "managers/messagemanager.h"

class MessageModel;
class IMessageDataManager;
class TokenManager;
struct Message;
struct User;
struct ReactionInfo;

using MessageModelPtr = std::shared_ptr<MessageModel>;
using Token = QString;

class MessageUseCase : public QObject{
  Q_OBJECT
 public:
  using MessageModelPtr = std::shared_ptr<MessageModel>;
  using Token = QString;

  MessageUseCase(IMessageDataManager *message_data_manager, std::unique_ptr<MessageManager> message_manager,
                 TokenManager *token_manager);
  [[nodiscard]] MessageModel *getMessageModel(long long chat_id);
  [[nodiscard]] QList<Message> getChatMessages(long long chat_id, int limit = 20);
  void getChatMessagesAsync(long long chat_id);
  void deleteMessage(const Message &msg);

 Q_SIGNALS:
  void messageAdded(const Message &);

 private:
  IMessageDataManager *data_manager_;
  std::unique_ptr<MessageManager> message_manager_;
  TokenManager *token_manager_;
};

#endif  // MESSAGEUSECASE_H
