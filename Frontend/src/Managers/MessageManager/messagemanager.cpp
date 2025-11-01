#include "messagemanager.h"

#include <QEventLoop>
#include <QUrl>
#include <QUrlQuery>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>

#include "DebugProfiling/Debug_profiling.h"
#include "headers/INetworkAccessManager.h"
#include "headers/JsonService.h"

namespace {

auto getRequestWithToken(QUrl endpoint, QString current_token) -> QNetworkRequest {
  auto request = QNetworkRequest(endpoint);
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
  request.setRawHeader("Authorization", current_token.toUtf8());
  return request;
}

}  // namespace

MessageManager::MessageManager(INetworkAccessManager* network_manager, QUrl url)
    : network_manager_(network_manager)
    , url_(url) { }

QList<Message> MessageManager::getChatMessages(QString current_token, int chat_id, int before_id, int limit) {
  PROFILE_SCOPE("ChatManager::getChatMessages");

  QUrl url("http://localhost:8082");
  QUrl endpoint = url.resolved(QUrl(QString("/messages/%1").arg(chat_id)));
  LOG_INFO("For chatId '{}' limit is '{}' and beforeId '{}'", chat_id, limit,
           before_id);
  QUrlQuery query;
  query.addQueryItem("limit", QString::number(limit));
  query.addQueryItem("beforeId", QString::number(before_id));
  endpoint.setQuery(query);
  auto request = getRequestWithToken(endpoint, current_token);
  auto* reply = network_manager_->get(request);

  QEventLoop loop;
  QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
  loop.exec();

  return onGetChatMessages(reply);
}

auto MessageManager::onGetChatMessages(QNetworkReply* reply) -> QList<Message> {
  PROFILE_SCOPE("ChatManager::onGetChatMessages");
  QScopedPointer<QNetworkReply, QScopedPointerDeleteLater> guard(reply);

  if (reply->error() != QNetworkReply::NoError) {
    LOG_ERROR("[onGetChatMessages] Network error: '{}'",
              reply->errorString().toStdString());
    //Q_EMIT errorOccurred("[network] " + reply->errorString());
    return {};
  }

  auto doc = QJsonDocument::fromJson(reply->readAll());
  if (!doc.isArray()) {
    LOG_ERROR("[onGetChatMessages] Invalid JSON: expected array");
    //Q_EMIT errorOccurred("Invalid JSON: expected array at root");
    return {};
  }

  QList<Message> messages;
  for (const auto& val : doc.array()) {
    messages.append(JsonService::getMessageFromJson(val.toObject()));
  }
  LOG_INFO("[onGetChatMessages] Loaded {} messages", messages.size());
  return messages;
}
