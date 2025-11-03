#ifndef MESSAGEMANAGER_H
#define MESSAGEMANAGER_H

#include <QList>
#include <QObject>
#include <QUrl>
#include <QFuture>

#include "headers/Message.h"
#include "headers/ChatBase.h"
#include "headers/BaseManager.h"

using ChatPtr = std::shared_ptr<ChatBase>;

class INetworkAccessManager;
class QNetworkReply;

class MessageManager : public BaseManager {
    Q_OBJECT
  public:
    using BaseManager::BaseManager;

    QFuture<QList<Message>> getChatMessages(QString current_token, int chat_id, int before_id, int limit);

  protected:
    QList<Message> onGetChatMessages(QNetworkReply* reply);
};

#endif // MESSAGEMANAGER_H
