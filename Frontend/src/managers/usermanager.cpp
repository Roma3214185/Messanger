#include "managers/usermanager.h"

#include <QEventLoop>
#include <QJsonArray>
#include <QNetworkReply>
#include <QPromise>
#include <QTimer>

#include "Debug_profiling.h"
#include "JsonService.h"
#include "interfaces/INetworkAccessManager.h"

namespace {

auto getRequestWithToken(const QUrl &endpoint, const QString &current_token) -> QNetworkRequest {
  auto request = QNetworkRequest(endpoint);
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
  request.setRawHeader("Authorization", current_token.toUtf8());
  return request;
}

}  // namespace

QFuture<std::optional<User>> UserManager::getUser(long long user_id, const QString &current_token) {
  LOG_INFO("[getUser] Loading user id={}", user_id);
  PROFILE_SCOPE("UserManager::getUser");

  QUrl endpoint = url_.resolved(QUrl(QString("/users/%1").arg(user_id)));
  auto req = getRequestWithToken(endpoint, current_token);
  auto *reply = network_manager_->get(req);

  return handleReplyWithTimeout<std::optional<User>>(
      reply, [this](const QByteArray &responce_data) { return onGetUser(responce_data); }, timeout_ms_, std::nullopt);
}

auto UserManager::onGetUser(const QByteArray &responce_data) const -> std::optional<User> {
  PROFILE_SCOPE("UserManager::onGetUser");
  // if(!checkReply(reply)) return std::nullopt;
  // QScopedPointer<QNetworkReply, QScopedPointerDeleteLater> guard(reply);

  auto doc = QJsonDocument::fromJson(responce_data);
  if (!doc.isObject()) {
    LOG_ERROR("[onGetUser] Invalid JSON: expected object at root");
    Q_EMIT errorOccurred("Invalid JSON: expected object at root");
    return std::nullopt;
  }

  auto user = this->entity_factory_->getUserFromResponse(doc.object());
  LOG_INFO("[onGetUser] User loaded: '{}'", user.name.toStdString());
  return user;
}

QFuture<QList<User>> UserManager::findUsersByTag(const QString &tag, const QString &current_token) {
  PROFILE_SCOPE("UserManager::findUsersByTag");
  LOG_INFO("[findUsersByTag] Searching for users with tag={}", tag.toStdString());

  QUrl endpoint = url_.resolved(QUrl(QString("/users/search?tag=%1").arg(tag)));
  auto request = getRequestWithToken(endpoint, current_token);

  auto *reply = network_manager_->get(request);

  return handleReplyWithTimeout<QList<User>>(
      reply, [this](const QByteArray &responce_data) { return onFindUsersByTag(responce_data); }, timeout_ms_,
      QList<User>{});
}

QList<User> UserManager::onFindUsersByTag(const QByteArray &responce_data) const {
  auto doc = QJsonDocument::fromJson(responce_data);

  if (!doc.isObject()) {
    LOG_ERROR("Invalid JSON: expected object at root");
    Q_EMIT errorOccurred("Invalid JSON: expected object at root");
    return {};
  }

  auto rootObj = doc.object();
  if (!rootObj.contains("users")) {
    LOG_ERROR("Invalid JSON: no 'users' field");
    Q_EMIT errorOccurred("Invalid JSON: no 'users' field");
    return {};
  }

  auto arr = rootObj["users"].toArray();
  QList<User> users;

  for (const auto &value : std::as_const(arr)) {
    auto obj = value.toObject();
    auto user = this->entity_factory_->getUserFromResponse(obj);
    users.append(user);
  }

  return users;
}
