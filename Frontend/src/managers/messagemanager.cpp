#include "managers/messagemanager.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QUrlQuery>

#include "Debug_profiling.h"
#include "JsonService.h"
#include "interfaces/INetworkAccessManager.h"

MessageManager::MessageManager(IMessageJsonService *entity_factory, INetworkAccessManager *network_manager,
                               const QUrl &base_url, std::chrono::milliseconds timeout_ms, QObject *parent)
    : entity_factory_(entity_factory), BaseManager(network_manager, base_url, timeout_ms, parent) {}

QFuture<QList<Message>> MessageManager::getChatMessages(const QString &current_token, long long chat_id,
                                                        long long before_id, long long limit) {
  PROFILE_SCOPE();
  LOG_INFO("For chatId '{}' limit is '{}' and beforeId '{}'", chat_id, limit, before_id);
  QUrl endpoint = url_.resolved(QUrl(QString("/messages/%1").arg(chat_id)));
  QUrlQuery query;
  query.addQueryItem("limit", QString::number(limit));
  query.addQueryItem("before_id", QString::number(before_id));
  endpoint.setQuery(query);
  auto request = this->getRequestWithToken(endpoint, current_token);
  auto *reply = network_manager_->get(request);

  return handleReplyWithTimeout<QList<Message>>(
      reply, [this](const QByteArray &responce_data) { return onGetChatMessages(responce_data); }, timeout_ms_,
      QList<Message>{});
}

QList<Message> MessageManager::onGetChatMessages(const QByteArray &responce_data) {
  PROFILE_SCOPE();
  auto doc = QJsonDocument::fromJson(responce_data);
  if (!doc.isArray()) {
    Q_EMIT errorOccurred("Invalid JSON: expected array at root");
    return QList<Message>{};
  }

  auto messages = QList<Message>{};
  auto reactions_infos = std::unordered_set<ReactionInfo>{};
  for (const auto &val : doc.array()) {
    auto [message, reactions] = this->entity_factory_->getMessageFromJson(val.toObject());
    messages.append(message);
    for (const auto &reaction : reactions) {
      reactions_infos.insert(reaction);
    }
  }

  for (const auto &reaction : reactions_infos) Q_EMIT saveReactionInfo(reaction);
  LOG_INFO("[onGetChatMessages] Loaded {} messages", messages.size());
  return messages;
}

void MessageManager::updateMessage(const Message &message_to_update, const QString &token) {
  PROFILE_SCOPE();
  DBC_REQUIRE(message_to_update.checkInvariants());

  QUrl endpoint = url_.resolved(QUrl(QString("/messages/%1").arg(message_to_update.id)));
  LOG_INFO("Update message {}", message_to_update.toString());
  QUrlQuery query;
  endpoint.setQuery(query);
  QJsonObject json = this->entity_factory_->toJson(message_to_update);

  auto request = this->getRequestWithToken(endpoint, token);
  auto *reply = network_manager_->put(request, QJsonDocument(json).toJson());
}

void MessageManager::deleteMessage(const Message &message_to_delete, const QString &token) {
  PROFILE_SCOPE();
  DBC_REQUIRE(message_to_delete.checkInvariants());
  QUrl endpoint = url_.resolved(QUrl(QString("/messages/%1").arg(message_to_delete.id)));
  auto request = this->getRequestWithToken(endpoint, token);
  auto *reply = network_manager_->del(request);
}
