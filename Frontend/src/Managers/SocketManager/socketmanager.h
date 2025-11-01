#ifndef SOCKETMANAGER_H
#define SOCKETMANAGER_H

#include <QUrl>
#include <QObject>

class QWebSocket;

class SocketManager : public QObject {
    Q_OBJECT
  public:
    SocketManager(QWebSocket* socket, QUrl url);
    void connectSocket(int user_id);
    void close();
    void sendText(const QString& message);

  private:
    void onSocketConnected(int user_id);

    QWebSocket* socket_;
    QUrl url_;

  Q_SIGNALS:
    void newTextFromSocket(const QString& text);
};

#endif // SOCKETMANAGER_H
