#include "managers/usermanager.h"

#include <QEventLoop>
#include <QJsonArray>
#include <QNetworkReply>
#include <QPromise>
#include <QTimer>

#include "Debug_profiling.h"
#include "JsonService.h"
#include "interfaces/INetworkAccessManager.h"

UserManager::UserManager(IUserJsonService *entity_factory, INetworkAccessManager *network_manager, const QUrl &base_url,
                         std::chrono::milliseconds timeout_ms, QObject *parent)
    : entity_factory_(entity_factory), BaseManager(network_manager, base_url, timeout_ms, parent) {}

QFuture<std::optional<User>> UserManager::getUser(long long user_id, const QString &current_token) {
  PROFILE_SCOPE();
  LOG_INFO("[getUser] Loading user id={}", user_id);
  QUrl endpoint = url_.resolved(QUrl(QString("/users/%1").arg(user_id)));
  auto req = getRequestWithToken(endpoint, current_token);
  auto *reply = network_manager_->get(req);

  return handleReplyWithTimeout<std::optional<User>>(
      reply, [this](const QByteArray &responce_data) { return onGetUser(responce_data); }, timeout_ms_, std::nullopt);
}

auto UserManager::onGetUser(const QByteArray &responce_data) const -> std::optional<User> {
  PROFILE_SCOPE();
  auto doc = QJsonDocument::fromJson(responce_data);
  if (!doc.isObject()) {
    Q_EMIT errorOccurred("Invalid JSON: expected object at root");
    return std::nullopt;
  }

  auto user = this->entity_factory_->getUserFromResponse(doc.object());
  LOG_INFO("User loaded: '{}'", user.name.toStdString());
  return user;
}

QFuture<QList<User>> UserManager::findUsersByTag(const QString &tag, const QString &current_token) {
  PROFILE_SCOPE();
  LOG_INFO("Searching for users with tag={}", tag.toStdString());
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
    Q_EMIT errorOccurred("Invalid JSON: expected object at root");
    return {};
  }

  auto rootObj = doc.object();
  if (!rootObj.contains("users")) {
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
