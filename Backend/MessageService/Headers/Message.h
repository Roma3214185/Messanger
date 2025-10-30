#ifndef BACKEND_MESSAGESERVICE_HEADERS_MESSAGE_H_
#define BACKEND_MESSAGESERVICE_HEADERS_MESSAGE_H_

#include <QDateTime>
#include <QString>
#include <cstdint>
#include <nlohmann/json.hpp>
#include <string>
#include <tuple>

#include "Meta.h"

struct Message {
  long long id;
  long long chat_id;
  long long sender_id;
  long long timestamp;
  std::string text;
  std::string local_id;
};

template <>
struct Reflection<Message> {
  static Meta meta() {
    return Meta{
        .name = "Messages",
        .table_name = "messages",
        .fields = {
            make_field<Message, long long>("id", &Message::id),
            make_field<Message, long long>("sender_id", &Message::sender_id),
            make_field<Message, long long>("chat_id", &Message::chat_id),
            make_field<Message, std::string>("text", &Message::text),
            make_field<Message, long long>("timestamp", &Message::timestamp),
            make_field<Message, std::string>("local_id", &Message::local_id)}
    };
  }
};

template <>
struct Builder<Message> {
  static Message build(QSqlQuery& query) {
    Message message;
    int idx = 0;

    auto assign = [&](auto& field) -> void {
      using TField = std::decay_t<decltype(field)>;
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

inline constexpr auto MessageFields =
    std::make_tuple(&Message::id, &Message::chat_id, &Message::sender_id,
                    &Message::text, &Message::timestamp);

template <>
struct EntityFields<Message> {
  static constexpr auto& fields = MessageFields;
};

[[nodiscard]] inline nlohmann::json to_json(const Message& message) {
  auto json_message = nlohmann::json{{"id", message.id},
                                {"chat_id", message.chat_id},
                                {"sender_id", message.sender_id},
                                {"text", message.text},
                                {"timestamp", message.timestamp},
                                {"local_id", message.local_id}};
  return json_message;
}

inline void to_json(nlohmann::json& json_message, const Message& message) {
  json_message = nlohmann::json{
      {"id", message.id},
      {"chat_id", message.chat_id},
      {"sender_id", message.sender_id},
      {"text", message.text},
      {"timestamp", message.timestamp},
      {"local_id", message.local_id}
  };
}

inline void from_json(const nlohmann::json& json_message, Message& message) {
  json_message.at("id").get_to(message.id);
  json_message.at("chat_id").get_to(message.chat_id);
  json_message.at("sender_id").get_to(message.sender_id);
  json_message.at("text").get_to(message.text);
  json_message.at("timestamp").get_to(message.timestamp);
  json_message.at("local_id").get_to(message.local_id);
}

#endif  // BACKEND_MESSAGESERVICE_HEADERS_MESSAGE_H_
