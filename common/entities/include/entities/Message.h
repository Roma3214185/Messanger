#ifndef BACKEND_MESSAGESERVICE_HEADERS_MESSAGE_H_
#define BACKEND_MESSAGESERVICE_HEADERS_MESSAGE_H_

#include <crow/crow.h>

#include <QDateTime>
#include <QString>
#include <cstdint>
#include <nlohmann/json.hpp>
#include <string>
#include <tuple>

#include "Meta.h"

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
    return Meta{.name       = "Messages",
                .table_name = "messages",
                .fields     = {make_field<Message, long long>("id", &Message::id),
                               make_field<Message, long long>("sender_id", &Message::sender_id),
                               make_field<Message, long long>("chat_id", &Message::chat_id),
                               make_field<Message, std::string>("text", &Message::text),
                               make_field<Message, long long>("timestamp", &Message::timestamp),
                               make_field<Message, std::string>("local_id", &Message::local_id)}};
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
    json_message = nlohmann::json{{"id", message.id},
                                  {"chat_id", message.chat_id},
                                  {"sender_id", message.sender_id},
                                  {"text", message.text},
                                  {"timestamp", message.timestamp},
                                  {"local_id", message.local_id}};
  }

  static void from_json(const nlohmann::json& json_message, Message& message) {
    json_message.at("id").get_to(message.id);
    json_message.at("chat_id").get_to(message.chat_id);
    json_message.at("sender_id").get_to(message.sender_id);
    json_message.at("text").get_to(message.text);
    json_message.at("timestamp").get_to(message.timestamp);
    json_message.at("local_id").get_to(message.local_id);
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
  json_message["id"]        = message.id;
  json_message["chat_id"]   = message.chat_id;
  json_message["sender_id"] = message.sender_id;
  json_message["text"]      = message.text;
  json_message["timestamp"] =
      QDateTime::fromSecsSinceEpoch(message.timestamp).toString(Qt::ISODate).toStdString();
  json_message["local_id"] = message.local_id;
  LOG_INFO("Local_id for text {} is {}", message.text, message.local_id);
  return json_message;
}

inline Message from_crow_json(const crow::json::rvalue& json_message) {
  Message message;
  if (json_message.count("id")) {
    message.id = json_message["id"].i();
  } else {
    message.id = 0;
  }

  message.chat_id   = json_message["chat_id"].i();
  message.sender_id = json_message["sender_id"].i();
  message.text      = json_message["text"].s();
  message.local_id  = json_message["local_id"].s();
  LOG_INFO("[Message] For text: {}, Local_id = ", message.text, message.local_id);

  if (json_message.count("timestamp")) {
    QString   timestamp = QString::fromStdString(json_message["timestamp"].s());
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
