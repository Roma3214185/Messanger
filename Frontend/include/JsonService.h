#ifndef ENTITY_FACTORY_H
#define ENTITY_FACTORY_H

#include <QDateTime>
#include <QJsonArray>
#include <QJsonObject>
#include <QString>

#include "dto/ChatBase.h"
#include "dto/Message.h"
#include "dto/User.h"
#include "entities/ReactionInfo.h"
#include "managers/TokenManager.h"

using MessageServerJsonAnswer = std::pair<Message, std::vector<ReactionInfo>>;

class EntityFactory {
  TokenManager *token_manager_;

 public:
  explicit EntityFactory(TokenManager *token_manager);

  Message createMessage(long long chat_id, long long sender_id, std::vector<MessageToken> tokens,
                        const QString &local_id, std::optional<long long> answer_on = std::nullopt,
                        QDateTime timestamp = QDateTime::currentDateTime());

  User getUserFromResponse(const QJsonObject &res);
  MessageServerJsonAnswer getMessageFromJson(const QJsonObject &obj);
  QJsonObject toJson(const Message &msg);
  ChatPtr getChatFromJson(const QJsonObject &obj);
  Reaction getReaction(const QJsonObject &obj);
  std::optional<ReactionInfo> getReactionInfo(const QJsonValue &value);
};

#endif  // ENTITY_FACTORY_H
