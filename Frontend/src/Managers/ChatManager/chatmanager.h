#ifndef CHATMANAGER_H
#define CHATMANAGER_H

#include <QList>
#include <QObject>
#include <QUrl>
#include <QFuture>

#include "headers/ChatBase.h"
#include "headers/IManager.h"

using ChatPtr = std::shared_ptr<ChatBase>;

class INetworkAccessManager;
class QNetworkReply;

class ChatManager : public IManager {
    Q_OBJECT
  public:
    ChatManager(INetworkAccessManager* network_manager, const QUrl& url, int timeout_ms = 5000);
    QFuture<QList<ChatPtr>> loadChats(const QString& current_token);
    QFuture<ChatPtr> loadChat(const QString& current_token, int chat_id);
    QFuture<ChatPtr> createPrivateChat(const QString& current_token, int user_id);

  protected:
    QList<ChatPtr> onLoadChats(QNetworkReply* reply);
    ChatPtr onChatLoaded(QNetworkReply* reply);
    ChatPtr onCreatePrivateChat(QNetworkReply* reply);

  private:
    int timeout_ms_;
    INetworkAccessManager* network_manager_;
    QUrl url_;
};




#endif // CHATMANAGER_H
