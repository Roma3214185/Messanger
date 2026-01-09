#include "managers/messagemanager.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QUrlQuery>

#include "Debug_profiling.h"
#include "JsonService.h"
#include "interfaces/INetworkAccessManager.h"

namespace {

auto getRequestWithToken(QUrl endpoint, QString current_token) -> QNetworkRequest {
  auto request = QNetworkRequest(endpoint);
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
  request.setRawHeader("Authorization", current_token.toUtf8());
  return request;
}

}  // namespace

QFuture<QList<Message>> MessageManager::getChatMessages(const QString &current_token, long long chat_id,
                                                        long long before_id, long long limit) {
  PROFILE_SCOPE("MessageManager::getChatMessages");

  QUrl endpoint = url_.resolved(QUrl(QString("/messages/%1").arg(chat_id)));
  LOG_INFO("For chatId '{}' limit is '{}' and beforeId '{}'", chat_id, limit, before_id);
  QUrlQuery query;
  query.addQueryItem("limit", QString::number(limit));
  query.addQueryItem("before_id", QString::number(before_id));
  endpoint.setQuery(query);
  auto request = getRequestWithToken(endpoint, current_token);
  auto *reply = network_manager_->get(request);
  // TODO(roma): make function getReplyGetChatMessages();

  return handleReplyWithTimeout<QList<Message>>(
      reply, [this](const QByteArray &responce_data) { return onGetChatMessages(responce_data); }, timeout_ms_,
      QList<Message>{});
}

QList<Message> MessageManager::onGetChatMessages(const QByteArray &responce_data) {
  PROFILE_SCOPE("ChatManager::onGetChatMessages");
  // QScopedPointer<QNetworkReply, QScopedPointerDeleteLater> guard(reply);

  // if (!reply || reply->error() != QNetworkReply::NoError) {
  //   LOG_ERROR("[onGetChatMessages] Network error: '{}'",
  //   reply->errorString().toStdString()); Q_EMIT errorOccurred("[network] " +
  //   reply->errorString()); return QList<Message>{};
  // }

  auto doc = QJsonDocument::fromJson(responce_data);
  if (!doc.isArray()) {
    LOG_ERROR("[onGetChatMessages] Invalid JSON: expected array");
    Q_EMIT errorOccurred("Invalid JSON: expected array at root");
    return QList<Message>{};
  }

  QList<Message> messages;
  for (const auto &val : doc.array()) {
    messages.append(JsonService::getMessageFromJson(val.toObject()));
  }
  LOG_INFO("[onGetChatMessages] Loaded {} messages", messages.size());
  return messages;
}

void MessageManager::updateMessage(const Message &message_to_update, const QString &token) {
  PROFILE_SCOPE("MessageManager::updateMessage");
  DBC_REQUIRE(message_to_update.checkInvariants());

  QUrl endpoint = url_.resolved(QUrl(QString("/messages/%1").arg(message_to_update.id)));
  LOG_INFO("Update message {}", message_to_update.toString());
  QUrlQuery query;
  endpoint.setQuery(query);
  QJsonObject json = JsonService::toJson(message_to_update);

  auto request = getRequestWithToken(endpoint, token);
  auto *reply = network_manager_->put(request, QJsonDocument(json).toJson());
}

void MessageManager::deleteMessage(const Message &message_to_delete, const QString &token) {
  PROFILE_SCOPE("MessageManager::getChatMessages");
  DBC_REQUIRE(message_to_delete.checkInvariants());

  QUrl endpoint = url_.resolved(QUrl(QString("/messages/%1").arg(message_to_delete.id)));
  LOG_INFO("Delete message {}", message_to_delete.toString());
  auto request = getRequestWithToken(endpoint, token);
  auto *reply = network_manager_->del(request);
}
