#ifndef SOCKETUSECASE_H
#define SOCKETUSECASE_H

#include <QObject>

#include "managers/socketmanager.h"

class Message;

class SocketUseCase : public QObject {
    Q_OBJECT
  public:
    SocketUseCase(SocketManager* socket_manager);
    void initSocket(long long user_id);
    void connectSocket();
    void sendMessage(const Message& msg);

  Q_SIGNALS:
    //void chatAdded(long long id);
    void errorOccurred(const QString& error);
    void newResponce(QJsonObject& message);

  private:
    void onMessageReceived(const QString& msg);

    SocketManager* socket_manager_;
};

#endif // SOCKETUSECASE_H
