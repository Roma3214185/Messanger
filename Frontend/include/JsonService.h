#ifndef JSONSERVER_H
#define JSONSERVER_H

#include <QDateTime>
#include <QJsonObject>
#include <QString>

#include "Debug_profiling.h"
#include "dto/User.h"
#include "dto/Message.h"
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
  auto msg = Message{
    .id = obj["id"].toInteger(),
    .sender_id = obj["sender_id"].toInteger(),
    .chat_id = obj["chat_id"].toInteger(),
    .text =   obj["text"].toString(),
    .timestamp =   QDateTime::fromSecsSinceEpoch(obj["timestamp"].toInteger()),
    .readed_by_me =  obj["readed_by_me"].toBool(false),
    .liked_by_me =  false,
    .read_counter =  0,
    .liked_counter =  0,
    .status_sended =   true,
    .local_id =   obj["local_id"].toString()
  };

  spdlog::info(msg.toString());
  return msg;
}

inline auto getChatFromJson(const QJsonObject& obj) -> ChatPtr {
  if(!obj.contains("id")) {
    LOG_ERROR("There is no id field");
    return nullptr;
  }

  if(!obj.contains("type")) {
    LOG_ERROR("There is no type field");
    return nullptr;
  }

  const QString type = obj["type"].toString();

  if (type == "private") {
    const auto userObj = obj["user"].toObject();
    auto       chat    = std::make_shared<PrivateChat>();
    chat->chat_id      = static_cast<long long>(obj["id"].toDouble());
    chat->title        = userObj["name"].toString();
    chat->avatar_path  = userObj["avatar"].toString();
    chat->user_id      = static_cast<long long>(userObj["id"].toDouble());
    LOG_INFO("Load private chat: {} and id {}", chat->title.toStdString(), chat->chat_id);
    //todo: check invariants
    return chat;
  } else if (type == "group") {
    auto chat          = std::make_shared<GroupChat>();
    chat->chat_id      = static_cast<long long>(obj["id"].toDouble());
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
