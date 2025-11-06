#ifndef USERMANAGER_H
#define USERMANAGER_H

#include <QList>
#include <QUrl>
#include <QFuture>

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

    QFuture<QList<User>> findUsersByTag(const QString& tag);
    QFuture<std::optional<User>> getUser(int user_id);

  protected:
    QList<User> onFindUsersByTag(QNetworkReply* reply);
    std::optional<User> onGetUser(QNetworkReply* reply);
};

#endif // USERMANAGER_H
