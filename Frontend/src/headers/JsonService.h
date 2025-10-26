#ifndef JSONSERVER_H
#define JSONSERVER_H

#include <QDateTime>
#include <QJsonObject>
#include <QString>

#include "ChatModel/chatmodel.h"
#include "Debug_profiling.h"
#include "MessageModel/messagemodel.h"
#include "headers/User.h"

using ChatPtr = std::shared_ptr<ChatBase>;

namespace JsonService {

inline auto getUserFromResponse(const QJsonObject& res) -> User {
  User user{.email = res["email"].toString(),
         .tag = res["tag"].toString(),
         .name = res["name"].toString(),
         .id = res["id"].toInt()};
  spdlog::info("[USER] id={} | name='{}' | tag='{}' | email='{}'", user.id,
               user.name.toStdString(), user.tag.toStdString(),
               user.email.toStdString());
  return user;
}

inline auto getMessageFromJson(const QJsonObject& obj) -> Message {
  Message msg{.id = obj["id"].toInt(),
              .senderId = obj["sender_id"].toInt(),
              .chatId = obj["chat_id"].toInt(),
              .text = obj["text"].toString(),
              .timestamp = QDateTime::fromString(obj["timestamp"].toString(),
                                                 Qt::ISODate),
              .readed_by_me = obj["readed_by_me"].toBool(false)};

  spdlog::info(
      "[MESSAGE] id={} | "
      "senderId='{}' | "
      "chatId='{}' | "
      "text='{}' | "
      "timestamp='{}' | readed_by_me = '{}'",
      msg.id, msg.senderId, msg.chatId, msg.text.toStdString(),
      msg.timestamp.toString().toStdString(), msg.readed_by_me);
  return msg;
}

inline auto getChatFromJson(const QJsonObject& obj) -> ChatPtr {
  const QString type = obj["type"].toString();

  if (type == "private") {
    const auto userObj = obj["user"].toObject();
    auto chat = std::make_shared<PrivateChat>();
    chat->chatId = obj["id"].toInt();
    chat->title = userObj["name"].toString();
    chat->avatarPath = userObj["avatar"].toString();
    chat->userId = userObj["id"].toInt();
    qDebug() << "[INFO] Load private chat:" << chat->title
             << "id:" << chat->chatId;
    return chat;
  } else if(type == "group"){
    auto chat = std::make_shared<GroupChat>();
    chat->chatId = obj["id"].toInt();
    chat->title = obj["name"].toString();
    chat->avatarPath = obj["avatar"].toString();
    chat->memberCount = obj["member_count"].toInt();
    qDebug() << "[INFO] Load group chat:" << chat->title
             << "id:" << chat->chatId;
    return chat;
  }
  return nullptr;
}

inline auto getPrivateChatFromJson(const QJsonObject& obj) -> ChatPtr {
  auto chat = std::make_shared<PrivateChat>();
  chat->chatId = obj["chat_id"].toInt();
  chat->userId = obj["user_id"].toInt();
  chat->title = obj["title"].toString();
  chat->avatarPath = obj["avatar"].toString();
  return chat;
}

}  // namespace JsonService

#endif  // JSONSERVER_H
