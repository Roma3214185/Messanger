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

ChatManager::ChatManager(IChatJsonService *entity_factory, INetworkAccessManager *network_manager, const QUrl &base_url,
                         std::chrono::milliseconds timeout_ms, QObject *parent)
    : entity_factory_(entity_factory), BaseManager(network_manager, base_url, timeout_ms, parent) {}

QFuture<QList<ChatPtr>> ChatManager::loadChats(const QString &current_token) {
  PROFILE_SCOPE();
  QUrl endpoint = url_.resolved(QUrl("/chats"));
  auto req = getRequestWithToken(endpoint, current_token);

  auto *reply = network_manager_->get(req);
  return handleReplyWithTimeout<QList<ChatPtr>>(
      reply, [this](const QByteArray &responce_data) { return onLoadChats(responce_data); }, timeout_ms_,
      QList<ChatPtr>{});
}

auto ChatManager::onLoadChats(const QByteArray &responce_data) -> QList<ChatPtr> {
  PROFILE_SCOPE();
  auto doc = QJsonDocument::fromJson(responce_data);
  if (!doc.isObject() || !doc.object().contains("chats") || !doc.object()["chats"].isArray()) {
    Q_EMIT errorOccurred("LoadChats: invalid JSON");
    return {};
  }

  auto chats = QList<ChatPtr>{};
  auto chats_array = doc.object()["chats"].toArray();
  for (const auto &chat_json : chats_array) {
    if (auto chat = this->entity_factory_->getChatFromJson(chat_json.toObject()); chat != nullptr) {
      chats.append(chat);
    } else {
      LOG_WARN("Skipping invalid chat object");
    }
  }

  LOG_INFO("Loaded {} chats", chats.size());
  return chats;
}

QFuture<ChatPtr> ChatManager::loadChat(const QString &current_token, long long chat_id) {
  PROFILE_SCOPE();
  QUrl endpoint = url_.resolved(QUrl(QString("/chats/%1").arg(chat_id)));
  auto req = getRequestWithToken(endpoint, current_token);

  auto *reply = network_manager_->get(req);
  return handleReplyWithTimeout<ChatPtr>(
      reply, [this](const QByteArray &responce_data) { return onChatLoaded(responce_data); }, timeout_ms_, nullptr);
}

ChatPtr ChatManager::onChatLoaded(const QByteArray &responce_data) {
  PROFILE_SCOPE();
  auto doc = QJsonDocument::fromJson(responce_data);
  if (!doc.isObject()) {
    Q_EMIT errorOccurred("loadChat: invalid JSON root");
    return nullptr;
  }

  return this->entity_factory_->getChatFromJson(doc.object());
}

QFuture<ChatPtr> ChatManager::createPrivateChat(const QString &current_token, long long user_id) {
  PROFILE_SCOPE();
  auto endpoint = url_.resolved(QUrl("/chats/private"));
  auto req = getRequestWithToken(endpoint, current_token);
  auto body = QJsonObject{{"user_id", user_id}};
  auto reply = network_manager_->post(req, QJsonDocument(body).toJson());
  return handleReplyWithTimeout<ChatPtr>(
      reply, [this](const QByteArray &responce_data) { return onCreatePrivateChat(responce_data); }, timeout_ms_,
      nullptr);
}

auto ChatManager::onCreatePrivateChat(const QByteArray &responce_data) -> ChatPtr {
  PROFILE_SCOPE();
  auto doc = QJsonDocument::fromJson(responce_data);
  if (!doc.isObject()) {
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
