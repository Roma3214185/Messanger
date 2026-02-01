#ifndef SOCKETUSECASE_H
#define SOCKETUSECASE_H

#include <QObject>

#include "managers/socketmanager.h"

struct Message;
struct Reaction;
struct MessageStatus;

class ISocketUseCase : public QObject {
  Q_OBJECT
 public:
  virtual ~ISocketUseCase() = default;
  virtual void initSocket(long long user_id) = 0;
  virtual void connectSocket() = 0;
  virtual void sendMessage(const Message &msg) = 0;
  virtual void sendReadMessageEvent(const MessageStatus &message_status) = 0;
  virtual void close() = 0;
  virtual void saveReaction(const Reaction &reaction) = 0;
  virtual void deleteReaction(const Reaction &reaction) = 0;
  virtual void sendInSocket(const QString &text) = 0;
  virtual void sendInSocket(const QJsonObject &text) = 0;

 private:
  virtual void onMessageReceived(const QString &msg);

 Q_SIGNALS:
  // void chatAdded(long long id);
  void errorOccurred(const QString &error);
  void newResponce(QJsonObject &message);
};

class SocketUseCase : public ISocketUseCase {
  Q_OBJECT
 public:
  explicit SocketUseCase(std::unique_ptr<SocketManager> socket_manager);
  void initSocket(long long user_id) override;
  void connectSocket() override;
  void sendMessage(const Message &msg) override;
  void sendReadMessageEvent(const MessageStatus &message_status) override;
  void close() override { socket_manager_->close(); }
  void saveReaction(const Reaction &reaction) override;
  void deleteReaction(const Reaction &reaction) override;
  void sendInSocket(const QString &text) override;
  void sendInSocket(const QJsonObject &text) override;

 private:
  void onMessageReceived(const QString &msg) override;

  std::unique_ptr<SocketManager> socket_manager_;
};

#endif  // SOCKETUSECASE_H
