#ifndef MESSAGEMANAGER_H
#define MESSAGEMANAGER_H

#include <QList>
#include <QObject>
#include <QUrl>

#include "headers/Message.h"
#include "headers/ChatBase.h"

using ChatPtr = std::shared_ptr<ChatBase>;

class INetworkAccessManager;
class QNetworkReply;

class MessageManager {
  public:
    MessageManager(INetworkAccessManager* network_manager, QUrl url);
    QList<Message> getChatMessages(QString current_token, int chat_id, int before_id, int limit);

  private:
    QList<Message> onGetChatMessages(QNetworkReply* reply);

    INetworkAccessManager* network_manager_;
    QUrl url_;
};


#endif // MESSAGEMANAGER_H
