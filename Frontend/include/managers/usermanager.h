#ifndef USERMANAGER_H
#define USERMANAGER_H

#include <QFuture>
#include <QList>
#include <QUrl>

#include "dto/User.h"
#include "managers/BaseManager.h"

class INetworkAccessManager;
class QUrl;
class QNetworkReply;
class User;

class UserManager : public BaseManager {
  Q_OBJECT
 public:
  using BaseManager::BaseManager;

  QFuture<QList<User>>         findUsersByTag(const QString& tag, const QString& current_token);
  QFuture<std::optional<User>> getUser(int user_id, const QString& current_token);

 protected:
  QList<User>         onFindUsersByTag(const QByteArray& responce_data) const;
  std::optional<User> onGetUser(const QByteArray& responce_data) const;
};

#endif  // USERMANAGER_H
