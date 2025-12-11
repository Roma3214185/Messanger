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

auto getRequestWithToken(QUrl endpoint, const QString& current_token) -> QNetworkRequest {
  auto request = QNetworkRequest(endpoint);
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
  request.setRawHeader("Authorization", current_token.toUtf8());
  return request;
}

}  // namespace

QFuture<QList<ChatPtr>> ChatManager::loadChats(const QString& current_token) {
  PROFILE_SCOPE("Model::loadChats");
  QUrl            endpoint = url_.resolved(QUrl("/chats"));
  auto req = getRequestWithToken(endpoint, current_token);

  auto* reply = network_manager_->get(req);
  return handleReplyWithTimeout<QList<ChatPtr>>(
      reply,
      [this](QNetworkReply* server_reply) { return onLoadChats(server_reply); },
      timeout_ms_,
      QList<ChatPtr>{});
}

auto ChatManager::onLoadChats(QNetworkReply* reply) -> QList<ChatPtr> {
  PROFILE_SCOPE("Model::onLoadChats");
  QScopedPointer<QNetworkReply, QScopedPointerDeleteLater> guard(reply);

  if (!reply || reply->error() != QNetworkReply::NoError) {
    LOG_ERROR("[onLoadChats] Network error: '{}'", reply->errorString().toStdString());
    Q_EMIT errorOccurred(reply->errorString());
    return {};
  }

  QByteArray raw = reply->readAll();
  spdlog::warn("[onLoadChats] RAW RESPONSE ({} bytes):\n{}", raw.size(), raw.toStdString());

  auto doc = QJsonDocument::fromJson(raw);
  if (!doc.isObject() || !doc.object().contains("chats") || !doc.object()["chats"].isArray()) {
    LOG_ERROR("LoadChats: invalid JSON");
    Q_EMIT errorOccurred("LoadChats: invalid JSON");
    return {};
  }

  auto chats = QList<ChatPtr>{};
  for (const auto& val : doc.object()["chats"].toArray()) {
    auto chat = JsonService::getChatFromJson(val.toObject());
    if (chat)
      chats.append(chat);
    else
      spdlog::warn("[onLoadChats] Skipping invalid chat object");
  }

  LOG_INFO("[onLoadChats] Loaded {} chats", chats.size());
  return chats;
}

QFuture<ChatPtr> ChatManager::loadChat(const QString& current_token, int chat_id) {
  PROFILE_SCOPE("Model::loadChat");
  QUrl            endpoint = url_.resolved(QUrl(QString("/chats/%1").arg(chat_id)));
  auto req = getRequestWithToken(endpoint, current_token);

  auto* reply = network_manager_->get(req);
  return handleReplyWithTimeout<ChatPtr>(
      reply,
      [this](QNetworkReply* server_reply) { return onChatLoaded(server_reply); },
      timeout_ms_,
      nullptr);
}

ChatPtr ChatManager::onChatLoaded(QNetworkReply* reply) {
  PROFILE_SCOPE("Model::onChatLoaded");

  if (!reply || reply->error() != QNetworkReply::NoError) {
    LOG_ERROR("[onChatLoaded] Network error: '{}'", reply->errorString().toStdString());
    Q_EMIT errorOccurred(reply->errorString());
    return nullptr;
  }

  QScopedPointer<QNetworkReply, QScopedPointerDeleteLater> guard(reply);

  auto doc = QJsonDocument::fromJson(reply->readAll());
  if (!doc.isObject()) {
    LOG_ERROR("[onChatLoaded] Invalid JSON, expected object at root");
    Q_EMIT errorOccurred("loadChat: invalid JSON root");
    return nullptr;
  }

  auto chat = JsonService::getChatFromJson(doc.object());
  return chat;
}

QFuture<ChatPtr> ChatManager::createPrivateChat(const QString& current_token, int user_id) {
  PROFILE_SCOPE("Model::createPrivateChat");
  auto endpoint = url_.resolved(QUrl("/chats/private"));
  auto req = getRequestWithToken(endpoint, current_token);
  auto body = QJsonObject{
      {"user_id", user_id},
  };
  auto reply = network_manager_->post(req, QJsonDocument(body).toJson());
  return handleReplyWithTimeout<ChatPtr>(
      reply,
      [this](QNetworkReply* server_reply) { return onCreatePrivateChat(server_reply); },
      timeout_ms_,
      nullptr);
}

auto ChatManager::onCreatePrivateChat(QNetworkReply* reply) -> ChatPtr {
  PROFILE_SCOPE("Model::onCreatePrivateChat");
  //QScopedPointer<QNetworkReply, QScopedPointerDeleteLater> guard(reply);
  if (!reply || reply->error() != QNetworkReply::NoError) {
    LOG_ERROR("[onCreatePrivateChat] error '{}'", reply->errorString().toStdString());
    Q_EMIT errorOccurred("onCreatePrivateChat" + reply->errorString());
    return nullptr;
  }

  auto responseData = reply->readAll();
  auto doc          = QJsonDocument::fromJson(responseData);
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

  auto new_chat = JsonService::getChatFromJson(responseObj);
  LOG_INFO("Private chat created with id '{}' ", new_chat->chat_id);
  return new_chat;
}
