#include "JsonService.h"

#include <QDateTime>
#include <QJsonArray>
#include <QJsonObject>
#include <QString>

#include "Debug_profiling.h"
#include "dto/ChatBase.h"
#include "dto/Message.h"
#include "dto/User.h"
#include "entities/MessageStatus.h"
#include "entities/ReactionInfo.h"
#include "managers/TokenManager.h"

Message MessageFactory::createMessage(long long current_user_id, long long chat_id, long long sender_id,
                                      std::vector<MessageToken> tokens, const QString &local_id,
                                      std::optional<long long> answer_on, QDateTime timestamp) {
  DBC_REQUIRE(!tokens.empty());
  DBC_REQUIRE(!local_id.isEmpty());
  DBC_REQUIRE(sender_id > 0);
  DBC_REQUIRE(chat_id > 0);
  // todo: what about message_id??
  Message message{.sender_id = sender_id,
                  .chat_id = chat_id,
                  .tokens = std::move(tokens),
                  .timestamp = std::move(timestamp),
                  .status_sended = false,
                  .receiver_id = current_user_id,
                  .local_id = local_id,
                  .answer_on = answer_on};

  // if(message.isMine()) {
  //   message.receiver_read_status = true;
  //   message.read_counter++;
  // }
  DBC_ENSURE(message.checkInvariants());
  return message;
}

JsonService::JsonService(TokenManager *token_manager) : token_manager_(token_manager) {}

User JsonService::getUserFromResponse(const QJsonObject &res) {
  if (!res.contains("email")) LOG_ERROR("No email field");
  if (!res.contains("tag")) LOG_ERROR("No tag field");
  if (!res.contains("name")) LOG_ERROR("No name field");
  if (!res.contains("id")) LOG_ERROR("No id field");
  if (!res.contains("avatar_path")) LOG_ERROR("No avatar_path field");

  User user;
  user.email = res["email"].toString();
  user.tag = res["tag"].toString();
  user.name = res["name"].toString();
  user.id = res["id"].toInteger();
  user.avatarPath = res["avatar_path"].toString("/Users/roma/QtProjects/Chat/images/default_avatar.jpg");

  spdlog::info("[USER] id={} | name='{}' | tag='{}' | email='{}'", user.id, user.name.toStdString(),
               user.tag.toStdString(), user.email.toStdString());
  DBC_ENSURE(user.checkInvariants());
  return user;
}

IMessageJsonService::MessageServerJsonAnswer JsonService::getMessageFromJson(const QJsonObject &obj) {
  Message msg;
  std::vector<ReactionInfo> reactions_infos;
  if (obj.contains("id")) msg.id = obj["id"].toInteger();
  if (obj.contains("sender_id")) msg.sender_id = obj["sender_id"].toInteger();
  if (obj.contains("chat_id")) msg.chat_id = obj["chat_id"].toInteger();
  if (obj.contains("text")) {
    QString text = obj["text"].toString();
    msg.tokens = utils::text::get_tokens_from_text(text);
  }

  if (obj.contains("answer_on")) {
    msg.answer_on = obj["answer_on"].toInteger();
  } else {
    msg.answer_on.reset();
  }

  if (obj.contains("timestamp")) msg.timestamp = QDateTime::fromSecsSinceEpoch(obj["timestamp"].toInteger());
  if (obj.contains("local_id")) msg.local_id = obj["local_id"].toString();
  if (obj.contains("read")) {
    const QJsonObject &read = obj["read"].toObject();
    if (read.contains("receiver_read_status")) msg.receiver_read_status = read["receiver_read_status"].toBool();
    if (read.contains("count")) msg.read_counter = read["count"].toInt();
  }

  if (obj.contains("reactions")) {
    const QJsonObject &react = obj["reactions"].toObject();
    if (react.contains("receiver_reaction") && !react["receiver_reaction"].isNull()) {
      msg.receiver_reaction = react["receiver_reaction"].toInt();
    } else {
      msg.receiver_reaction.reset();
    }

    if (react.contains("counts") && react["counts"].isArray()) {
      const QJsonArray countsArr = react["counts"].toArray();

      msg.reactions.clear();

      for (const auto &v : countsArr) {
        if (!v.isArray()) continue;  // skip invalid entries

        QJsonArray pair = v.toArray();
        if (pair.size() != 2) continue;
        int count = pair[1].toInt();

        if (auto reaction_info = getReactionInfo(pair[0]); reaction_info.has_value()) {
          msg.reactions[reaction_info->id]++;
          reactions_infos.push_back(reaction_info.value());
        } else {
          LOG_ERROR("Unable to get reactionInfo from JsonValue");
        }
      }
    }
  }

  msg.receiver_id = token_manager_->getCurrentUserId();
  msg.status_sended = true;
  DBC_ENSURE(msg.checkInvariants());

  LOG_INFO("[JSON] {}", msg.toString());
  return std::make_pair(msg, std::move(reactions_infos));
}

QJsonObject JsonService::toJson(const Message &msg) {
  QJsonObject obj;
  obj["id"] = msg.id;
  obj["sender_id"] = msg.sender_id;
  obj["chat_id"] = msg.chat_id;
  obj["text"] = msg.getFullText();
  obj["timestamp"] = msg.timestamp.toSecsSinceEpoch();
  obj["local_id"] = msg.local_id;

  QJsonObject readObj;
  readObj["receiver_read_status"] = msg.receiver_read_status;
  readObj["count"] = msg.read_counter;
  obj["read"] = readObj;

  // QJsonObject reactionsObj;
  // reactionsObj["counts"] = ...;
  // reactionsObj["my_reaction"] = ...;  // int or null
  // obj["reactions"] = reactionsObj;
  // obj["status_sended"] = msg.status_sended;
  return obj;
}

ChatPtr JsonService::getChatFromJson(const QJsonObject &obj) {
  if (!obj.contains("id")) {
    LOG_ERROR("There is no id field");
    return nullptr;
  }

  if (!obj.contains("type")) {
    LOG_ERROR("There is no type field");
    return nullptr;
  }

  const QString type = obj["type"].toString();
  ChatPtr chat = nullptr;

  if (type == "private") {
    const auto userObj = obj["user"].toObject();
    chat = std::make_shared<PrivateChat>();
    // chat = dynamic_cast<PrivateChat>(chat);
    chat->chat_id = static_cast<long long>(obj["id"].toDouble());
    chat->title = userObj["name"].toString();
    chat->avatar_path = userObj["avatar"].toString();
    // chat->user_id = static_cast<long long>(userObj["id"].toDouble());
    LOG_INFO("Load private chat: {} and id {}", chat->title.toStdString(), chat->chat_id);
    // todo: check invariants
  } else if (type == "group") {
    chat = std::make_shared<GroupChat>();
    chat->chat_id = static_cast<long long>(obj["id"].toDouble());
    chat->title = obj["name"].toString();
    chat->avatar_path = obj["avatar"].toString();
    // chat->member_count = obj["member_count"].toInt();
    LOG_INFO("Load group chat: {} and id {}", chat->title.toStdString(), chat->chat_id);
  } else {
    return nullptr;
  }

  chat->default_reactions.clear();

  if (obj.contains("default_reactions")) {
    const auto reactArr = obj["default_reactions"].toArray();

    for (const auto &v : reactArr) {
      if (auto reaction_info = getReactionInfo(v); reaction_info.has_value()) {
        chat->default_reactions.push_back(reaction_info.value());
      } else {
        LOG_ERROR("Unable to get reactionInfo from JsonValue");
      }
    }
  }

  return chat;
}

Reaction JsonService::getReaction(const QJsonObject &obj) {
  if (!obj.contains("reaction_id")) LOG_ERROR("Obj for reaction doesn't contains 'reaction_id' field");
  if (!obj.contains("message_id")) LOG_ERROR("Obj for reaction doesn't contains 'message_id' field");
  if (!obj.contains("receiver_id")) LOG_ERROR("Obj for reaction doesn't contains 'receiver_id' field");

  Reaction reaction;
  reaction.reaction_id = obj["reaction_id"].toInteger();
  reaction.message_id = obj["message_id"].toInteger();
  reaction.receiver_id = obj["receiver_id"].toInteger();
  DBC_ENSURE(reaction.checkInvariants());
  return reaction;
}

std::optional<ReactionInfo> JsonService::getReactionInfo(const QJsonValue &value) {
  if (value.isObject() == false) return std::nullopt;
  const auto reaction_object = value.toObject();
  ReactionInfo reaction_info;
  reaction_info.id = static_cast<long long>(reaction_object["id"].toDouble());
  reaction_info.image = reaction_object["image"].toString().toStdString();
  return reaction_info.checkInvariants() ? std::make_optional(reaction_info) : std::nullopt;
}

std::optional<MessageStatus> JsonService::getMessageStatus(const QJsonObject &json_object) {
  MessageStatus status;
  if (!json_object.contains("message_id")) {
    LOG_ERROR("getMessageStatus doen't have field message_id");
    return std::nullopt;
  }

  if (!json_object.contains("receiver_id")) {
    LOG_ERROR("getMessageStatus doen't have field receiver_id");
    return std::nullopt;
  }

  status.message_id = json_object["message_id"].toInteger();
  status.receiver_id = json_object["receiver_id"].toInteger();
  status.is_read = true;  // todo: implememnt correct json from server
  // status.is_read = json_object["is_read"].toBool();
  // status.read_at = json_object["is_read"].toInteger();

  // todo: maybe already checkInvariants call and return std::nullopt if incorrect ??
  return status;
}
