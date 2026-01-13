#ifndef ENTITY_FACTORY_H
#define ENTITY_FACTORY_H

#include <QDateTime>
#include <QJsonObject>
#include <QString>
#include <QJsonArray>

#include "Debug_profiling.h"
#include "dto/ChatBase.h"
#include "dto/Message.h"
#include "dto/User.h"
#include "managers/TokenManager.h"

class EntityFactory {
  TokenManager *token_manager_;

 public:
  EntityFactory(TokenManager *token_manager) : token_manager_(token_manager) {}

  Message createMessage(long long chat_id, long long sender_id, const QString &text, const QString &local_id,
                        QDateTime timestamp = QDateTime::currentDateTime()) {
    DBC_REQUIRE(!text.isEmpty());
    DBC_REQUIRE(!local_id.isEmpty());
    DBC_REQUIRE(sender_id > 0);
    DBC_REQUIRE(chat_id > 0);
    // todo: what about message_id??
    Message message{.chat_id = chat_id,
                    .sender_id = sender_id,
                    .text = text,
                    .receiver_id = token_manager_->getCurrentUserId(),
                    .status_sended = false,
                    .timestamp = timestamp,
                    .local_id = local_id};

    // if(message.isMine()) {
    //   message.receiver_read_status = true;
    //   message.read_counter++;
    // }
    DBC_ENSURE(message.checkInvariants());
    return message;
  }

  User getUserFromResponse(const QJsonObject &res) {
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
    return user;
  }

  Message getMessageFromJson(const QJsonObject &obj) {
    Message msg;
    if (obj.contains("id")) msg.id = obj["id"].toInteger();
    if (obj.contains("sender_id")) msg.sender_id = obj["sender_id"].toInteger();
    if (obj.contains("chat_id")) msg.chat_id = obj["chat_id"].toInteger();
    if (obj.contains("text")) msg.text = obj["text"].toString();
    if (obj.contains("timestamp")) msg.timestamp = QDateTime::fromSecsSinceEpoch(obj["timestamp"].toInteger());
    if (obj.contains("local_id")) msg.local_id = obj["local_id"].toString();
    if (obj.contains("read")) {
      const QJsonObject &read = obj["read"].toObject();
      if (read.contains("receiver_read_status")) msg.receiver_read_status = read["receiver_read_status"].toBool();
      if (read.contains("count")) msg.read_counter = read["count"].toInt();
    }

    if (obj.contains("reactions")) {
      const QJsonObject &react = obj["reactions"].toObject();
      if (react.contains("receiver_reaction"))
        msg.receiver_reaction = react["receiver_reaction"].toInt();
      else
        msg.receiver_reaction.reset();

      if (react.contains("counts") && react["counts"].isArray()) {
        const QJsonArray countsArr = react["counts"].toArray();

        msg.reactions.clear();

        for (const QJsonValue &v : countsArr) {
          if (!v.isArray()) continue;  // skip invalid entries

          QJsonArray pair = v.toArray();
          if (pair.size() != 2) continue;

          int reaction_id = pair[0].toInt();
          int count = pair[1].toInt();

          msg.reactions[reaction_id] = count;
        }
      }
    }

    msg.receiver_id = token_manager_->getCurrentUserId();
    // if (msg.isMine()) {
    //   DBC_REQUIRE(msg.read_counter > 0);
    //   DBC_REQUIRE(msg.receiver_read_status == true);
    //   // msg.receiver_read_status = true;
    //   // msg.read_counter++;
    // }
    msg.status_sended = true;

    LOG_INFO("[JSON] {}", msg.toString());
    return msg;
  }

  QJsonObject toJson(const Message &msg) {
    QJsonObject obj;
    obj["id"] = msg.id;
    obj["sender_id"] = msg.sender_id;
    obj["chat_id"] = msg.chat_id;
    obj["text"] = msg.text;
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

  ChatPtr getChatFromJson(const QJsonObject &obj) {
    if (!obj.contains("id")) {
      LOG_ERROR("There is no id field");
      return nullptr;
    }

    if (!obj.contains("type")) {
      LOG_ERROR("There is no type field");
      return nullptr;
    }

    const QString type = obj["type"].toString();

    if (type == "private") {
      const auto userObj = obj["user"].toObject();
      auto chat = std::make_shared<PrivateChat>();
      chat->chat_id = static_cast<long long>(obj["id"].toDouble());
      chat->title = userObj["name"].toString();
      chat->avatar_path = userObj["avatar"].toString();
      chat->user_id = static_cast<long long>(userObj["id"].toDouble());
      LOG_INFO("Load private chat: {} and id {}", chat->title.toStdString(), chat->chat_id);
      // todo: check invariants
      return chat;
    } else if (type == "group") {
      auto chat = std::make_shared<GroupChat>();
      chat->chat_id = static_cast<long long>(obj["id"].toDouble());
      chat->title = obj["name"].toString();
      chat->avatar_path = obj["avatar"].toString();
      chat->member_count = obj["member_count"].toInt();
      LOG_INFO("Load group chat: {} and id {}", chat->title.toStdString(), chat->chat_id);
      return chat;
    }
    return nullptr;
  }

  Reaction getReaction(const QJsonObject &obj) {
    if (obj.contains("reaction_id")) LOG_ERROR("Obj reaction doesn't contains 'reaction_id' field");
    if (obj.contains("message_id")) LOG_ERROR("Obj reaction doesn't contains 'message_id' field");
    if (obj.contains("receiver_id")) LOG_ERROR("Obj reaction doesn't contains 'receiver_id' field");

    Reaction reaction;
    reaction.reaction_id = obj["reaction_id"].toInteger();
    reaction.message_id = obj["message_id"].toInteger();
    reaction.receiver_id = obj["receiver_id"].toInteger();
    DBC_ENSURE(reaction.checkInvariants());
    return reaction;
  }
};

#endif  // ENTITY_FACTORY_H
