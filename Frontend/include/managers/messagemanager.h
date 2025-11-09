#ifndef MESSAGEMANAGER_H
#define MESSAGEMANAGER_H

#include <QFuture>
#include <QList>
#include <QObject>
#include <QUrl>

#include "dto/ChatBase.h"
#include "dto/Message.h"
#include "managers/BaseManager.h"

using ChatPtr = std::shared_ptr<ChatBase>;

class INetworkAccessManager;
class QNetworkReply;

class MessageManager : public BaseManager {
  Q_OBJECT
 public:
  using BaseManager::BaseManager;

  QFuture<QList<Message>> getChatMessages(const QString& current_token,
                                          int            chat_id,
                                          int            before_id,
                                          int            limit);

 protected:
  QList<Message> onGetChatMessages(QNetworkReply* reply);
};

#endif  // MESSAGEMANAGER_H
