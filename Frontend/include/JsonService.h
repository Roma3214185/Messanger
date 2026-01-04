#ifndef JSONSERVER_H
#define JSONSERVER_H

#include <QDateTime>
#include <QJsonObject>
#include <QString>

#include "Debug_profiling.h"
#include "dto/Message.h"
#include "dto/User.h"
#include "models/chatmodel.h"
#include "models/messagemodel.h"

using ChatPtr = std::shared_ptr<ChatBase>;

namespace JsonService {

inline auto getUserFromResponse(const QJsonObject& res) -> User {
  if (!res.contains("email")) LOG_ERROR("No email field");
  if (!res.contains("tag")) LOG_ERROR("No tag field");
  if (!res.contains("name")) LOG_ERROR("No name field");
  if (!res.contains("id")) LOG_ERROR("No id field");
  if (!res.contains("avatar_path")) LOG_ERROR("No avatar_path field");
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
  Message msg;
  if (obj.contains("id")) msg.id = obj["id"].toInteger();
  if (obj.contains("sender_id")) msg.sender_id = obj["sender_id"].toInteger();
  if (obj.contains("chat_id")) msg.chat_id = obj["chat_id"].toInteger();
  if (obj.contains("text")) msg.text = obj["text"].toString();
  if (obj.contains("timestamp"))
    msg.timestamp = QDateTime::fromSecsSinceEpoch(obj["timestamp"].toInteger());
  if (obj.contains("local_id")) msg.local_id = obj["local_id"].toString();
  if (obj.contains("read")) {
    const QJsonObject& read = obj["read"].toObject();
    if (read.contains("read_by_me")) msg.readed_by_me = read["read_by_me"].toBool();
    if (read.contains("count")) msg.read_counter = read["count"].toInt();
  }

  msg.liked_counter = 0;
  msg.status_sended = true;

  spdlog::info("[JSON] {}", msg.toString());
  return msg;
}

inline auto toJson(const Message& msg) -> QJsonObject {
  QJsonObject obj;
  obj["id"]        = msg.id;
  obj["sender_id"] = msg.sender_id;
  obj["chat_id"]   = msg.chat_id;
  obj["text"]      = msg.text;
  obj["timestamp"] = msg.timestamp.toSecsSinceEpoch();
  obj["local_id"]  = msg.local_id;

  QJsonObject readObj;
  readObj["read_by_me"] = msg.readed_by_me;
  readObj["count"]      = msg.read_counter;
  obj["read"]           = readObj;

  // QJsonObject reactionsObj;
  // reactionsObj["counts"] = ...;
  // reactionsObj["my_reaction"] = ...;  // int or null
  // obj["reactions"] = reactionsObj;
  // obj["status_sended"] = msg.status_sended;
  return obj;
}

inline auto getChatFromJson(const QJsonObject& obj) -> ChatPtr {
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
    auto       chat    = std::make_shared<PrivateChat>();
    chat->chat_id      = static_cast<long long>(obj["id"].toDouble());
    chat->title        = userObj["name"].toString();
    chat->avatar_path  = userObj["avatar"].toString();
    chat->user_id      = static_cast<long long>(userObj["id"].toDouble());
    LOG_INFO("Load private chat: {} and id {}", chat->title.toStdString(), chat->chat_id);
    // todo: check invariants
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

// static void to_json(nlohmann::json& j, const UserMessage& m) {
//   j = nlohmann::json(m.message);
//   j["read"] = {
//       {"count", m.read.count},
//       {"read_by_me", m.read.read_by_me}
//   };

//   j["reactions"]["counts"] = m.reactions.counts;

//   if (m.reactions.my_reaction.has_value())
//     j["reactions"]["my_reaction"] = *m.reactions.my_reaction;
//   else
//     j["reactions"]["my_reaction"] = nullptr;
// }

// static void from_json(const nlohmann::json& j, UserMessage& m) {
//   j.at("message").get_to(m.message);

//   if(j.contains("read")) {
//     if (j["read"].contains("read_by_me")) j["read"].at("read_by_me").get_to(m.read.read_by_me);
//     if (j["read"].contains("count")) j["read"].at("count").get_to(m.read.count);
//   }

//   if(j.contains("reactions")) {
//     if (j["reactions"].contains("counts"))
//     j["reactions"].at("counts").get_to(m.reactions.counts); if (j.contains("reactions") &&
//         j["reactions"].contains("my_reaction") &&
//         !j["reactions"]["my_reaction"].is_null())
//     {
//       m.reactions.my_reaction =
//           j["reactions"]["my_reaction"].get<int>();
//     }
//     else
//     {
//       m.reactions.my_reaction.reset();
//     }
//   }
// }
// };

// }  // namespace nlohmann
