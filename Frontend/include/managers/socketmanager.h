#ifndef SOCKETMANAGER_H
#define SOCKETMANAGER_H

#include <QObject>
#include <QUrl>

class ISocket;

class SocketManager : public QObject {
  Q_OBJECT
 public:
  SocketManager(ISocket *socket, const QUrl &url);
  void close();
  void sendText(const QString &message);
  void initSocket(long long user_id);
  void connectSocket();

 private:
  ISocket *socket_;
  QUrl url_;

 Q_SIGNALS:
  void newTextFromSocket(const QString &text);
};

#endif  // SOCKETMANAGER_H
