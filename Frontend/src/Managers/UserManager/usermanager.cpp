#include "usermanager.h"

#include <QEventLoop>
#include <QNetworkReply>
#include <QJsonArray>
#include <QTimer>

#include "DebugProfiling/Debug_profiling.h"
#include "headers/JsonService.h"
#include "headers/INetworkAccessManager.h"

UserManager::UserManager(INetworkAccessManager *network_manager, QUrl url)
    : network_manager_(network_manager)
    , url_(url) {}

void UserManager::getUser(
    int user_id,
    std::function<std::optional<User>(std::optional<User>)> onSuccess,
    std::function<std::optional<User>(QString)> onError) {
  PROFILE_SCOPE("UserManager::getUser");
  LOG_INFO("[getUser] Loading user id={}", user_id);

  QUrl endpoint = url_.resolved(QUrl(QString("/users/%1").arg(user_id)));
  QNetworkRequest req(endpoint);
  auto* reply = network_manager_->get(req);

  QTimer::singleShot(5000, reply, [reply, onError]() {
    if (reply->isRunning()) {
      reply->abort();
      onError("Server not respond");
    }
  });

  QObject::connect(reply, &QNetworkReply::finished, this,
                   [reply, onSuccess, onError, this]() mutable {
                     if (reply->error() != QNetworkReply::NoError) {
                       onError(reply->errorString());
                       reply->deleteLater();
                       return;
                     }

                     optional<User> user_ptr = onGetUser(reply);
                     reply->deleteLater();
                     onSuccess(user_ptr);
                   });
}

auto UserManager::onGetUser(QNetworkReply* reply) -> optional<User> {
  PROFILE_SCOPE("UserManager::onGetUser");
  QScopedPointer<QNetworkReply, QScopedPointerDeleteLater>
      guard(reply);

  if (reply->error() != QNetworkReply::NoError) {
    LOG_ERROR("[onGetUser] Network error: '{}'",
              reply->errorString().toStdString());
    //Q_EMIT errorOccurred("get user: " + reply->errorString());
    return std::nullopt;
  }

  auto doc = QJsonDocument::fromJson(reply->readAll());
  if (!doc.isObject()) {
    LOG_ERROR("[onGetUser] Invalid JSON: expected object at root");
    //Q_EMIT errorOccurred("Invalid JSON: expected object at root");
    return std::nullopt;
  }

  auto user = JsonService::getUserFromResponse(doc.object());
  LOG_INFO("[onGetUser] User loaded: '{}'", user.name.toStdString());
  return user;
}

void UserManager::findUsersByTag(
    const QString& tag,
    std::function<QList<User>(QList<User>)> onSuccess,
    std::function<QList<User>(QString)> onError)
{
  QUrl endpoint =
      url_.resolved(QUrl(QString("/users/search?tag=%1").arg(tag)));
  auto request = QNetworkRequest(endpoint);
  auto* reply = network_manager_->get(request);

  QTimer::singleShot(5000, reply, [reply, onError]() {
    if (reply->isRunning()) {
      reply->abort();
      onError("Server not respond");
    }
  });

  QObject::connect(reply, &QNetworkReply::finished, this,
                   [this, reply, onSuccess, onError]() mutable {
                     if (reply->error() != QNetworkReply::NoError) {
                       onError(reply->errorString());
                       reply->deleteLater();
                       return;
                     }

                     QList<User> users = onFindUsersByTag(reply);
                     reply->deleteLater();
                     onSuccess(users);
                   });
}

QList<User> UserManager::onFindUsersByTag(QNetworkReply* reply) {
  QScopedPointer<QNetworkReply, QScopedPointerDeleteLater> guard(reply);
  if (reply->error() != QNetworkReply::NoError) {
    //Q_EMIT errorOccurred("onFindUsers" + reply->errorString());
    return {};
  }
  auto responseData = reply->readAll();
  auto doc = QJsonDocument::fromJson(responseData);
  if (!doc.isObject()) {
    //Q_EMIT errorOccurred("Invalid JSON: expected object at root");
    return {};
  }
  auto rootObj = doc.object();
  auto arr = rootObj["users"].toArray();
  QList<User> users;
  for (const auto& value : std::as_const(arr)) {
    auto obj = value.toObject();
    auto user = JsonService::getUserFromResponse(obj);
    users.append(user);
  }
  return users;
}
