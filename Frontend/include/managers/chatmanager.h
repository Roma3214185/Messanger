#ifndef CHATMANAGER_H
#define CHATMANAGER_H

#include <QFuture>
#include <QList>
#include <QObject>

#include "managers/BaseManager.h"

using ChatPtr = std::shared_ptr<ChatBase>;

class INetworkAccessManager;
class QNetworkReply;
class IChatJsonService;
class QUrl;

class ChatManager : public BaseManager {
  Q_OBJECT
 public:
  ChatManager(IChatJsonService *, INetworkAccessManager *network_manager, const QUrl &base_url,
              std::chrono::milliseconds timeout_ms = std::chrono::milliseconds{500}, QObject *parent = nullptr);

  QFuture<QList<ChatPtr>> loadChats(const QString &current_token);
  QFuture<ChatPtr> loadChat(const QString &current_token, long long chat_id);
  QFuture<ChatPtr> createPrivateChat(const QString &current_token, long long user_id);

 protected:
  QList<ChatPtr> onLoadChats(const QByteArray &responce_data);
  ChatPtr onChatLoaded(const QByteArray &responce_data);
  ChatPtr onCreatePrivateChat(const QByteArray &responce_data);

 private:
  IChatJsonService *entity_factory_;
};

#endif  // CHATMANAGER_H
