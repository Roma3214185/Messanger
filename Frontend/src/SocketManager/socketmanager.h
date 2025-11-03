#ifndef SOCKETMANAGER_H
#define SOCKETMANAGER_H

#include <QUrl>
#include <QObject>

class ISocket;

class SocketManager : public QObject {
    Q_OBJECT
  public:
    SocketManager(ISocket* socket, const QUrl& url);
    void connectSocket(int user_id);
    void close();
    void sendText(const QString& message);

  protected:
    void onSocketConnected(int user_id);

  private:
    ISocket* socket_;
    QUrl url_;

  Q_SIGNALS:
    void newTextFromSocket(const QString& text);
};

#endif // SOCKETMANAGER_H
