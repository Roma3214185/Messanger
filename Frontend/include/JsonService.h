#ifndef JSONSERVER_H
#define JSONSERVER_H

#include <QDateTime>
#include <QJsonObject>
#include <QString>

#include "Debug_profiling.h"
#include "dto/User.h"
#include "models/chatmodel.h"
#include "models/messagemodel.h"

using ChatPtr = std::shared_ptr<ChatBase>;

namespace JsonService {

inline auto getUserFromResponse(const QJsonObject& res) -> User {
  if(!res.contains("email")) LOG_ERROR("No email field");
  if(!res.contains("tag")) LOG_ERROR("No tag field");
  if(!res.contains("name")) LOG_ERROR("No name field");
  if(!res.contains("id")) LOG_ERROR("No id field");
  if(!res.contains("avatar_path")) LOG_ERROR("No avatar_path field");
  User user;
  user.email      = res["email"].toString();
  user.tag        = res["tag"].toString();
  user.name       = res["name"].toString();
  user.id         = res["id"].toInteger();
  user.avatarPath = res["avatar_path"].toString();

  spdlog::info("[USER] id={} | name='{}' | tag='{}' | email='{}'",
               user.id,
               user.name.toStdString(),
               user.tag.toStdString(),
               user.email.toStdString());
  return user;
}

inline auto getMessageFromJson(const QJsonObject& obj) -> Message {
  Message msg{.id           = obj["id"].toInt(),
              .senderId     = obj["sender_id"].toInt(),
              .chatId       = obj["chat_id"].toInt(),
              .text         = obj["text"].toString(),
              .timestamp    = QDateTime::fromString(obj["timestamp"].toString(), Qt::ISODate),
              .readed_by_me = obj["readed_by_me"].toBool(false),
              .local_id     = obj["local_id"].toString()};

  spdlog::info(
      "[MESSAGE] id={} | "
      "senderId='{}' | "
      "chatId='{}' | "
      "text='{}' | "
      "timestamp='{}' | readed_by_me = '{}' | local_id = {}",
      msg.id,
      msg.senderId,
      msg.chatId,
      msg.text.toStdString(),
      msg.timestamp.toString().toStdString(),
      msg.readed_by_me,
      msg.local_id.toStdString());
  return msg;
}

inline auto getChatFromJson(const QJsonObject& obj) -> ChatPtr {
  const QString type = obj["type"].toString();

  if (type == "private") {
    const auto userObj = obj["user"].toObject();
    auto       chat    = std::make_shared<PrivateChat>();
    chat->chat_id      = obj["id"].toInt();
    chat->title        = userObj["name"].toString();
    chat->avatar_path  = userObj["avatar"].toString();
    chat->user_id      = userObj["id"].toInt();
    LOG_INFO("Load private chat: {} and id {}", chat->title.toStdString(), chat->chat_id);
    return chat;
  } else if (type == "group") {
    auto chat          = std::make_shared<GroupChat>();
    chat->chat_id      = obj["id"].toInt();
    chat->title        = obj["name"].toString();
    chat->avatar_path  = obj["avatar"].toString();
    chat->member_count = obj["member_count"].toInt();
    LOG_INFO("Load group chat: {} and id {}", chat->title.toStdString(), chat->chat_id);
    return chat;
  }
  return nullptr;
}

}  // namespace JsonService

#endif  // JSONSERVER_H
