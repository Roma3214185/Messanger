#ifndef SOCKETUSECASE_H
#define SOCKETUSECASE_H

#include <QObject>

#include "managers/socketmanager.h"

struct Message;
struct Reaction;
struct MessageStatus;

class SocketUseCase : public QObject {
  Q_OBJECT
 public:
  explicit SocketUseCase(std::unique_ptr<SocketManager> socket_manager);
  void initSocket(long long user_id);
  void connectSocket();
  void sendMessage(const Message &msg);
  void sendReadMessageEvent(const MessageStatus &message_status);
  void close();
  void saveReaction(const Reaction &reaction);
  void deleteReaction(const Reaction &reaction);
  void sendInSocket(const QString &text);
  void sendInSocket(const QJsonObject &text);

 Q_SIGNALS:
  // void chatAdded(long long id);
  void errorOccurred(const QString &error);
  void newResponce(QJsonObject &message);

 private:
  void onMessageReceived(const QString &msg);

  std::unique_ptr<SocketManager> socket_manager_;
};

#endif  // SOCKETUSECASE_H
