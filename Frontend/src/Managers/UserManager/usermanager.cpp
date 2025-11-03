#include "usermanager.h"

#include <QEventLoop>
#include <QNetworkReply>
#include <QJsonArray>
#include <QTimer>
#include <QPromise>

#include "DebugProfiling/Debug_profiling.h"
#include "headers/JsonService.h"
#include "headers/INetworkAccessManager.h"

const QString kServerNotRespondError = "Server didn't respond";

UserManager::UserManager(INetworkAccessManager *network_manager, const QUrl& url, int timeouts_ms)
    : network_manager_(network_manager)
    , url_(url)
    , timeouts_ms_(timeouts_ms) {}

QFuture<std::optional<User>> UserManager::getUser(int user_id) {
  PROFILE_SCOPE("UserManager::getUser");
  LOG_INFO("[getUser] Loading user id={}", user_id);

  QUrl endpoint = url_.resolved(QUrl(QString("/users/%1").arg(user_id)));
  QNetworkRequest req(endpoint);
  auto* reply = network_manager_->get(req);

  auto promisePtr = std::make_shared<QPromise<std::optional<User>>>();
  auto future = promisePtr->future();

  auto isCompleted = std::make_shared<std::atomic_bool>(false);

  QTimer::singleShot(timeouts_ms_, reply, [reply, promisePtr, isCompleted, this]() {
    if (isCompleted->exchange(true))
      return;

    if (reply->isRunning()) {
      reply->abort();
      Q_EMIT errorOccurred(kServerNotRespondError);
      promisePtr->addResult(std::nullopt);
      promisePtr->finish();
    }
  });

  QObject::connect(reply, &QNetworkReply::finished, this,
                   [reply, promisePtr, isCompleted, this]() mutable {
                     if (isCompleted->exchange(true)) {
                       reply->deleteLater();
                       return;
                     }

                     if (reply->error() != QNetworkReply::NoError) {
                       Q_EMIT errorOccurred("Error occurred: " + reply->errorString());
                       promisePtr->addResult(std::nullopt);
                     } else {
                       std::optional<User> userOpt = onGetUser(reply);
                       promisePtr->addResult(userOpt);
                     }

                     promisePtr->finish();
                     reply->deleteLater();
                   });

  return future;
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
    //Q_EMIT errorOccurred("Invalid JSON: expected object at root");
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

  auto promisePtr = std::make_shared<QPromise<QList<User>>>();
  auto future = promisePtr->future();

  auto isCompleted = std::make_shared<std::atomic_bool>(false);

  QTimer::singleShot(timeouts_ms_, reply, [reply, promisePtr, isCompleted, this]() {
    if (isCompleted->exchange(true))
      return;

    if (reply->isRunning()) {
      reply->abort();
      Q_EMIT errorOccurred(kServerNotRespondError);
      promisePtr->addResult({});
      promisePtr->finish();
    }
  });

  QObject::connect(reply, &QNetworkReply::finished, this,
                   [reply, promisePtr, isCompleted, this]() mutable {
                     if (isCompleted->exchange(true)) {
                       reply->deleteLater();
                       return;
                     }

                     if (reply->error() != QNetworkReply::NoError) {
                       Q_EMIT errorOccurred("Error occurred: " + reply->errorString());
                       promisePtr->addResult({});
                     } else {
                       QList<User> users = onFindUsersByTag(reply);
                       promisePtr->addResult(users);
                     }

                     promisePtr->finish();
                     reply->deleteLater();
                   });

  return future;
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
