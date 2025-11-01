#ifndef USERMANAGER_H
#define USERMANAGER_H

#include <QList>
#include <QUrl>

#include "headers/User.h"

class INetworkAccessManager;
class QUrl;
class QNetworkReply;
class User;

class UserManager {
  public:
    UserManager(INetworkAccessManager* network_manager, QUrl url);
    QList<User> findUsersByTag(const QString& tag);
    std::optional<User> getUser(int user_id);

  private:
    QList<User> findUsersByTag(QNetworkReply* reply);
    std::optional<User> onGetUser(QNetworkReply* reply);

    INetworkAccessManager* network_manager_;
    QUrl url_;
};

#endif // USERMANAGER_H
