#ifndef CHATMANAGER_H
#define CHATMANAGER_H

#include <QFuture>
#include <QList>
#include <QObject>
#include <QUrl>

#include "dto/ChatBase.h"
#include "managers/BaseManager.h"

using ChatPtr = std::shared_ptr<ChatBase>;

class INetworkAccessManager;
class QNetworkReply;

class ChatManager : public BaseManager {
  Q_OBJECT
 public:
  using BaseManager::BaseManager;

  QFuture<QList<ChatPtr>> loadChats(const QString& current_token);
  QFuture<ChatPtr>        loadChat(const QString& current_token, int chat_id);
  QFuture<ChatPtr>        createPrivateChat(const QString& current_token, int user_id);

 protected:
  QList<ChatPtr> onLoadChats(QNetworkReply* reply);
  ChatPtr        onChatLoaded(QNetworkReply* reply);
  ChatPtr        onCreatePrivateChat(QNetworkReply* reply);
};

#endif  // CHATMANAGER_H
