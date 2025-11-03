#ifndef CHATMANAGER_H
#define CHATMANAGER_H

#include <QList>
#include <QObject>
#include <QUrl>

#include "headers/ChatBase.h"

using ChatPtr = std::shared_ptr<ChatBase>;

class INetworkAccessManager;
class QNetworkReply;

class ChatManager {
  public:
    ChatManager(INetworkAccessManager* network_manager, QUrl url) : network_manager_(network_manager), url_(url) {}
    QList<ChatPtr> loadChats(const QString& current_token);
    ChatPtr loadChat(const QString& current_token, int chat_id);
    ChatPtr createPrivateChat(const QString& current_token, int user_id);

  protected:
    QList<ChatPtr> onLoadChats(QNetworkReply* reply);
    ChatPtr onChatLoaded(QNetworkReply* reply);
    ChatPtr onCreatePrivateChat(QNetworkReply* reply);

  private:
    INetworkAccessManager* network_manager_;
    QUrl url_;
};




#endif // CHATMANAGER_H
