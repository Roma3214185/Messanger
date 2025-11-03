#ifndef USERMANAGER_H
#define USERMANAGER_H

#include <QList>
#include <QUrl>
#include <QFuture>

#include "headers/User.h"
#include "headers/IManager.h"

class INetworkAccessManager;
class QUrl;
class QNetworkReply;
class User;

class UserManager : public IManager {
    Q_OBJECT
  public:
    UserManager(INetworkAccessManager* network_manager, const QUrl& url, int timeout_ms = 5000);
    QFuture<QList<User>> findUsersByTag(const QString& tag);
    QFuture<std::optional<User>> getUser(int user_id);

  Q_SIGNALS:
    void errorOccurred(const QString& error);

  protected:
    QList<User> onFindUsersByTag(QNetworkReply* reply);
    std::optional<User> onGetUser(QNetworkReply* reply);

  private:
    INetworkAccessManager* network_manager_;
    QUrl url_;
    int timeout_ms_;
};

#endif // USERMANAGER_H
