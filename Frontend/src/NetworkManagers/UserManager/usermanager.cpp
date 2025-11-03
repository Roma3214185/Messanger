#include "usermanager.h"

#include <QEventLoop>
#include <QNetworkReply>
#include <QJsonArray>
#include <QTimer>
#include <QPromise>

#include "DebugProfiling/Debug_profiling.h"
#include "headers/JsonService.h"
#include "headers/INetworkAccessManager.h"

QFuture<std::optional<User>> UserManager::getUser(int user_id) {
  PROFILE_SCOPE("UserManager::getUser");
  LOG_INFO("[getUser] Loading user id={}", user_id);

  QUrl endpoint = url_.resolved(QUrl(QString("/users/%1").arg(user_id)));
  QNetworkRequest req(endpoint);
  auto* reply = network_manager_->get(req);

  return handleReplyWithTimeout<std::optional<User>>(
      reply,
      [this](QNetworkReply* server_reply) { return onGetUser(server_reply); },
      timeout_ms_,
      std::nullopt
    );
}

auto UserManager::onGetUser(QNetworkReply* reply) -> optional<User> {
  PROFILE_SCOPE("UserManager::onGetUser");
  QScopedPointer<QNetworkReply, QScopedPointerDeleteLater>
      guard(reply);

  if (reply->error() != QNetworkReply::NoError) {
    LOG_ERROR("[onGetUser] Network error: '{}'",
              reply->errorString().toStdString());
    Q_EMIT errorOccurred("get user: " + reply->errorString());
    return std::nullopt;
  }

  auto doc = QJsonDocument::fromJson(reply->readAll());
  if (!doc.isObject()) {
    LOG_ERROR("[onGetUser] Invalid JSON: expected object at root");
    Q_EMIT errorOccurred("Invalid JSON: expected object at root");
    return std::nullopt;
  }

  auto user = JsonService::getUserFromResponse(doc.object());
  LOG_INFO("[onGetUser] User loaded: '{}'", user.name.toStdString());
  return user;
}

QFuture<QList<User>> UserManager::findUsersByTag(const QString& tag) {
  PROFILE_SCOPE("UserManager::findUsersByTag");
  LOG_INFO("[findUsersByTag] Searching for users with tag={}", tag.toStdString());

  QUrl endpoint = url_.resolved(QUrl(QString("/users/search?tag=%1").arg(tag)));
  QNetworkRequest request(endpoint);
  auto* reply = network_manager_->get(request);

  return handleReplyWithTimeout<QList<User>>(
    reply,
    [this](QNetworkReply* server_reply) { return onFindUsersByTag(server_reply); },
    timeout_ms_,
    QList<User>{}
  );
}

QList<User> UserManager::onFindUsersByTag(QNetworkReply* reply) {
  QScopedPointer<QNetworkReply, QScopedPointerDeleteLater> guard(reply);
  if (reply->error() != QNetworkReply::NoError) {
    Q_EMIT errorOccurred("onFindUsers" + reply->errorString());
    return {};
  }
  auto responseData = reply->readAll();
  auto doc = QJsonDocument::fromJson(responseData);

  if (!doc.isObject()) {
    Q_EMIT errorOccurred("Invalid JSON: expected object at root");
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
