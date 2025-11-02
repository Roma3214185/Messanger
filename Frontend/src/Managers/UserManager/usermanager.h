#ifndef USERMANAGER_H
#define USERMANAGER_H

#include <QList>
#include <QUrl>
#include <QObject>

#include "headers/User.h"

class INetworkAccessManager;
class QUrl;
class QNetworkReply;
class User;

class UserManager : public QObject {
    Q_OBJECT
  public:
    UserManager(INetworkAccessManager* network_manager, QUrl url);
    void findUsersByTag(
        const QString& tag,
        std::function<QList<User>(QList<User>)> onSuccess,
        std::function<QList<User>(QString)> onError);

   void getUser(
      int user_id,
      std::function<std::optional<User>(std::optional<User>)> onSuccess,
      std::function<std::optional<User>(QString)> onError);

  private:
    QList<User> onFindUsersByTag(QNetworkReply* reply);
    std::optional<User> onGetUser(QNetworkReply* reply);

    INetworkAccessManager* network_manager_;
    QUrl url_;
};

#endif // USERMANAGER_H
