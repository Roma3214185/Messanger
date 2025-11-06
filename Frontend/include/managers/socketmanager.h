#ifndef SOCKETMANAGER_H
#define SOCKETMANAGER_H

#include <QUrl>
#include <QObject>

class ISocket;

class SocketManager : public QObject {
    Q_OBJECT
  public:
    SocketManager(ISocket* socket, const QUrl& url);
    void close();
    void sendText(const QString& message);
    void initSocket(int user_id);
    void connectSocket();

  private:
    ISocket* socket_;
    QUrl url_;

  Q_SIGNALS:
    void newTextFromSocket(const QString& text);
};

#endif // SOCKETMANAGER_H
