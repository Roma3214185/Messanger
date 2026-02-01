#ifndef ENTITY_FACTORY_H
#define ENTITY_FACTORY_H

#include "dto/ChatBase.h"

class MessageStatus;
class TokenManager;
class User;
class ReactionInfo;
class Reaction;
class Message;
class ChatBase;

class IUserJsonService {
 public:
  virtual User getUserFromResponse(const QJsonObject &res) = 0;
  virtual ~IUserJsonService() = default;
};

class IMessageStatusJsonService {
 public:
  virtual std::optional<MessageStatus> getMessageStatus(const QJsonObject &res) = 0;
  virtual ~IMessageStatusJsonService() = default;
};

class IMessageJsonService {
 public:
  using MessageServerJsonAnswer = std::pair<Message, std::vector<ReactionInfo>>;
  virtual MessageServerJsonAnswer getMessageFromJson(const QJsonObject &obj) = 0;
  virtual QJsonObject toJson(const Message &msg) = 0;
  virtual ~IMessageJsonService() = default;
};

class IReactionJsonService {
 public:
  virtual Reaction getReaction(const QJsonObject &obj) = 0;
  virtual ~IReactionJsonService() = default;
};

class IReactionInfoJsonService {
 public:
  virtual std::optional<ReactionInfo> getReactionInfo(const QJsonValue &value) = 0;
  virtual ~IReactionInfoJsonService() = default;
};

class IChatJsonService {
 public:
  virtual ChatPtr getChatFromJson(const QJsonObject &obj) = 0;
  virtual ~IChatJsonService() = default;
};

class MessageFactory {
 public:
  static Message createMessage(long long current_user_id, long long chat_id, long long sender_id,
                               std::vector<MessageToken> tokens, const QString &local_id,
                               std::optional<long long> answer_on = std::nullopt,
                               QDateTime timestamp = QDateTime::currentDateTime());
};

class JsonService : public IUserJsonService,
                    public IMessageJsonService,
                    public IReactionJsonService,
                    public IReactionInfoJsonService,
                    public IChatJsonService,
                    public IMessageStatusJsonService {
  TokenManager *token_manager_;

 public:
  explicit JsonService(TokenManager *token_manager);

  User getUserFromResponse(const QJsonObject &res) override;
  MessageServerJsonAnswer getMessageFromJson(const QJsonObject &obj) override;
  QJsonObject toJson(const Message &msg) override;
  ChatPtr getChatFromJson(const QJsonObject &obj) override;
  Reaction getReaction(const QJsonObject &obj) override;
  std::optional<ReactionInfo> getReactionInfo(const QJsonValue &value) override;
  std::optional<MessageStatus> getMessageStatus(const QJsonObject &res) override;
};

#endif  // ENTITY_FACTORY_H
