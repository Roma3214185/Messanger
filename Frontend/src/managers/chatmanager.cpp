#include "managers/chatmanager.h"

#include <QFuture>
#include <QJsonArray>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QPromise>
#include <QTimer>
#include <QUrl>
#include <QUrlQuery>

#include "Debug_profiling.h"
#include "JsonService.h"
#include "interfaces/INetworkAccessManager.h"

const QString kServerNotRespondError = "Server didn't respond";

namespace {

auto getRequestWithToken(QUrl endpoint, const QString &current_token) -> QNetworkRequest {
  auto request = QNetworkRequest(endpoint);
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
  request.setRawHeader("Authorization", current_token.toUtf8());
  return request;
}

}  // namespace

QFuture<QList<ChatPtr>> ChatManager::loadChats(const QString &current_token) {
  PROFILE_SCOPE("Model::loadChats");
  QUrl endpoint = url_.resolved(QUrl("/chats"));
  auto req = getRequestWithToken(endpoint, current_token);

  auto *reply = network_manager_->get(req);
  return handleReplyWithTimeout<QList<ChatPtr>>(
      reply, [this](const QByteArray &responce_data) { return onLoadChats(responce_data); }, timeout_ms_,
      QList<ChatPtr>{});
}

auto ChatManager::onLoadChats(const QByteArray &responce_data) -> QList<ChatPtr> {
  PROFILE_SCOPE("Model::onLoadChats");
  // if(!checkReply(reply)) return {};
  // QScopedPointer<QNetworkReply, QScopedPointerDeleteLater> guard(reply);

  // QByteArray raw = reply->readAll();
  // spdlog::warn("[onLoadChats] RAW RESPONSE ({} bytes):\n{}",
  // responce_data.size(), responce_data.toStdString());

  auto doc = QJsonDocument::fromJson(responce_data);
  if (!doc.isObject() || !doc.object().contains("chats") || !doc.object()["chats"].isArray()) {
    LOG_ERROR("LoadChats: invalid JSON");
    Q_EMIT errorOccurred("LoadChats: invalid JSON");
    return {};
  }

  auto chats = QList<ChatPtr>{};
  for (const auto &val : doc.object()["chats"].toArray()) {
    auto chat = this->entity_factory_->getChatFromJson(val.toObject());
    if (chat)
      chats.append(chat);
    else
      spdlog::warn("[onLoadChats] Skipping invalid chat object");
  }

  LOG_INFO("[onLoadChats] Loaded {} chats", chats.size());
  return chats;
}

QFuture<ChatPtr> ChatManager::loadChat(const QString &current_token, long long chat_id) {
  PROFILE_SCOPE("Model::loadChat");
  QUrl endpoint = url_.resolved(QUrl(QString("/chats/%1").arg(chat_id)));
  auto req = getRequestWithToken(endpoint, current_token);

  auto *reply = network_manager_->get(req);
  return handleReplyWithTimeout<ChatPtr>(
      reply, [this](const QByteArray &responce_data) { return onChatLoaded(responce_data); }, timeout_ms_, nullptr);
}

ChatPtr ChatManager::onChatLoaded(const QByteArray &responce_data) {
  PROFILE_SCOPE("Model::onChatLoaded");
  // if(!checkReply(reply)) return nullptr;
  // QScopedPointer<QNetworkReply, QScopedPointerDeleteLater> guard(reply);

  auto doc = QJsonDocument::fromJson(responce_data);
  if (!doc.isObject()) {
    LOG_ERROR("[onChatLoaded] Invalid JSON, expected object at root");
    Q_EMIT errorOccurred("loadChat: invalid JSON root");
    return nullptr;
  }

  return this->entity_factory_->getChatFromJson(doc.object());
}

QFuture<ChatPtr> ChatManager::createPrivateChat(const QString &current_token, long long user_id) {
  PROFILE_SCOPE("Model::createPrivateChat");
  auto endpoint = url_.resolved(QUrl("/chats/private"));
  auto req = getRequestWithToken(endpoint, current_token);
  auto body = QJsonObject{
      {"user_id", user_id},
  };
  auto reply = network_manager_->post(req, QJsonDocument(body).toJson());
  return handleReplyWithTimeout<ChatPtr>(
      reply, [this](const QByteArray &responce_data) { return onCreatePrivateChat(responce_data); }, timeout_ms_,
      nullptr);
}

auto ChatManager::onCreatePrivateChat(const QByteArray &responce_data) -> ChatPtr {
  PROFILE_SCOPE("Model::onCreatePrivateChat");

  auto doc = QJsonDocument::fromJson(responce_data);
  if (!doc.isObject()) {
    LOG_ERROR("[onCreatePrivateChat] Invalid JSON: expected object at root");
    Q_EMIT errorOccurred("Invalid JSON: expected object at root");
    return nullptr;
  }

  auto responseObj = doc.object();
  if (responseObj["type"].toString() != "private") {
    Q_EMIT errorOccurred("Error in model create private chat returned group chat");
    return nullptr;
  }

  return this->entity_factory_->getChatFromJson(responseObj);
}
