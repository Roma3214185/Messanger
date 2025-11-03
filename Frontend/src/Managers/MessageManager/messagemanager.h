#ifndef MESSAGEMANAGER_H
#define MESSAGEMANAGER_H

#include <QList>
#include <QObject>
#include <QUrl>
#include <QFuture>

#include "headers/Message.h"
#include "headers/ChatBase.h"
#include "headers/IManager.h"

using ChatPtr = std::shared_ptr<ChatBase>;

class INetworkAccessManager;
class QNetworkReply;

class MessageManager : public IManager {
    Q_OBJECT
  public:
    MessageManager(INetworkAccessManager* network_manager, const QUrl& url, int timeout_ms = 5000);
    QFuture<QList<Message>> getChatMessages(QString current_token, int chat_id, int before_id, int limit);

  protected:
    QList<Message> onGetChatMessages(QNetworkReply* reply);

  private:
    INetworkAccessManager* network_manager_;
    QUrl url_;
    int timeout_ms_;
};

#endif // MESSAGEMANAGER_H
