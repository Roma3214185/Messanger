#ifndef CHATMANAGER_H
#define CHATMANAGER_H

#include <QFuture>
#include <QList>
#include <QObject>
#include <QUrl>

#include "managers/BaseManager.h"

using ChatPtr = std::shared_ptr<ChatBase>;

class INetworkAccessManager;
class QNetworkReply;

class ChatManager : public BaseManager {
  Q_OBJECT
 public:
  using BaseManager::BaseManager;

  QFuture<QList<ChatPtr>> loadChats(const QString &current_token);
  QFuture<ChatPtr> loadChat(const QString &current_token, long long chat_id);
  QFuture<ChatPtr> createPrivateChat(const QString &current_token, long long user_id);

 protected:
  QList<ChatPtr> onLoadChats(const QByteArray &responce_data);
  ChatPtr onChatLoaded(const QByteArray &responce_data);
  ChatPtr onCreatePrivateChat(const QByteArray &responce_data);
};

#endif  // CHATMANAGER_H
