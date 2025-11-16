#ifndef BACKEND_MESSAGESERVICE_HEADERS_MESSAGE_H_
#define BACKEND_MESSAGESERVICE_HEADERS_MESSAGE_H_

#include <crow.h>

#include <QDateTime>
#include <QString>
#include <cstdint>
#include <nlohmann/json.hpp>
#include <string>
#include <tuple>

#include "Meta.h"
#include "Fields.h"

struct Message {
  long long   id = 0;
  long long   chat_id;
  long long   sender_id;
  long long   timestamp;
  std::string text;
  std::string local_id;
};

template <>
struct Reflection<Message> {
  static Meta meta() {
    return Meta{.table_name = MessageTable::Table,
                .fields     = {make_field<Message, long long>(MessageTable::Id, &Message::id),
                               make_field<Message, long long>(MessageTable::SenderId, &Message::sender_id),
                               make_field<Message, long long>(MessageTable::ChatId, &Message::chat_id),
                               make_field<Message, std::string>(MessageTable::Text, &Message::text),
                               make_field<Message, long long>(MessageTable::Timestamp, &Message::timestamp),
                               make_field<Message, std::string>(MessageTable::LocalId, &Message::local_id)}};
  }
};

template <>
struct Builder<Message> {
  static Message build(QSqlQuery& query) {
    Message message;
    int     idx = 0;

    auto assign = [&](auto& field) -> void {
      using TField         = std::decay_t<decltype(field)>;
      const QVariant value = query.value(idx++);
      if constexpr (std::is_same_v<TField, long long>) {
        field = value.toLongLong();
      } else if constexpr (std::is_same_v<TField, int>) {
        field = value.toInt();
      } else if constexpr (std::is_same_v<TField, std::string>) {
        field = value.toString().toStdString();
      } else if constexpr (std::is_same_v<TField, QString>) {
        field = value.toString();
      } else {
        field = value.value<TField>();
      }
    };

    assign(message.id);
    assign(message.chat_id);
    assign(message.sender_id);
    assign(message.timestamp);
    assign(message.text);
    assign(message.local_id);

    return message;
  }
};

inline constexpr auto MessageFields = std::make_tuple(
    &Message::id, &Message::chat_id, &Message::sender_id, &Message::text, &Message::timestamp);

template <>
struct EntityFields<Message> {
  static constexpr auto& fields = MessageFields;
};

namespace nlohmann {

template <>
struct adl_serializer<Message> {
  static void to_json(nlohmann::json& json_message, const Message& message) {
    json_message = nlohmann::json{{MessageTable::Id, message.id},
                                  {MessageTable::ChatId, message.chat_id},
                                  {MessageTable::SenderId, message.sender_id},
                                  {MessageTable::Text, message.text},
                                  {MessageTable::Timestamp, message.timestamp},
                                  {MessageTable::LocalId, message.local_id}};
  }

  static void from_json(const nlohmann::json& json_message, Message& message) {
    json_message.at(MessageTable::Id).get_to(message.id);
    json_message.at(MessageTable::ChatId).get_to(message.chat_id);
    json_message.at(MessageTable::SenderId).get_to(message.sender_id);
    json_message.at(MessageTable::Text).get_to(message.text);
    json_message.at(MessageTable::Timestamp).get_to(message.timestamp);
    json_message.at(MessageTable::LocalId).get_to(message.local_id);
  }
};

}  // namespace nlohmann

inline crow::json::wvalue to_crow_json(const Message& message) {
  crow::json::wvalue json_message;
  LOG_INFO(
      "[Message] id '{}' | chat_id '{}' | sender_id '{}' | text '{}' | "
      "timestamp '{}'",
      message.id,
      message.chat_id,
      message.sender_id,
      message.text,
      message.timestamp);
  json_message[MessageTable::Id]        = message.id;
  json_message[MessageTable::ChatId]   = message.chat_id;
  json_message[MessageTable::SenderId] = message.sender_id;
  json_message[MessageTable::Text]      = message.text;
  json_message[MessageTable::Timestamp] =
      QDateTime::fromSecsSinceEpoch(message.timestamp).toString(Qt::ISODate).toStdString();
  json_message[MessageTable::LocalId] = message.local_id;
  LOG_INFO("Local_id for text {} is {}", message.text, message.local_id);
  return json_message;
}

inline Message from_crow_json(const crow::json::rvalue& json_message) {
  Message message;
  if (json_message.count(MessageTable::Id)) {
    message.id = json_message[MessageTable::Id].i();
  } else {
    message.id = 0;
  }

  message.chat_id   = json_message[MessageTable::ChatId].i();
  message.sender_id = json_message[MessageTable::SenderId].i();
  message.text      = json_message[MessageTable::Text].s();
  message.local_id  = json_message[MessageTable::LocalId].s();
  LOG_INFO("[Message] For text: {}, Local_id = ", message.text, message.local_id);

  if (json_message.count(MessageTable::Timestamp)) {
    QString   timestamp = QString::fromStdString(json_message[MessageTable::Timestamp].s());
    QDateTime datetime  = QDateTime::fromString(timestamp, Qt::ISODate);
    if (!datetime.isValid()) {
      message.timestamp = QDateTime::currentDateTime().toSecsSinceEpoch();
    } else {
      message.timestamp = datetime.toSecsSinceEpoch();
    }
  } else {
    message.timestamp = QDateTime::currentDateTime().toSecsSinceEpoch();
  }

  LOG_INFO(
      "[Message from json] id '{}' | chat_id '{}' | sender_id '{}' | text '{}' "
      "| timestamp '{}'",
      message.id,
      message.chat_id,
      message.sender_id,
      message.text,
      message.timestamp);
  return message;
}

#endif  // BACKEND_MESSAGESERVICE_HEADERS_MESSAGE_H_
