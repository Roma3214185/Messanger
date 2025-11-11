#ifndef BACKEND_NOTIFICATIONSERVICE_HEADERS_MESSAGE_H_
#define BACKEND_NOTIFICATIONSERVICE_HEADERS_MESSAGE_H_

#include <crow.h>

#include <QDateTime>
#include <nlohmann/json.hpp>
#include <string>

#include "Debug_profiling.h"

struct Message {
  long long   id;
  long long   chat_id;
  long long   sender_id;
  std::string text;
  long long   timestamp;
  std::string local_id;
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

inline Message from_crow_json(const crow::json::rvalue& j) {
  Message m;
  if (j.count("id"))
    m.id = j["id"].i();
  else
    m.id = 0;

  m.chat_id   = j["chat_id"].i();
  m.sender_id = j["sender_id"].i();
  m.text      = j["text"].s();
  m.local_id  = j["local_id"].s();

  if (j.count("timestamp")) {
    QString   ts = QString::fromStdString(j["timestamp"].s());
    QDateTime dt = QDateTime::fromString(ts, Qt::ISODate);
    if (!dt.isValid()) {
      m.timestamp = QDateTime::currentDateTime().toSecsSinceEpoch();
    } else {
      m.timestamp = dt.toSecsSinceEpoch();
    }
  } else {
    m.timestamp = QDateTime::currentDateTime().toSecsSinceEpoch();
  }

  LOG_INFO(
      "[Message from json] id '{}' | chat_id '{}' | sender_id '{}' | text '{}' "
      "| timestamp '{}' | local_id = {}",
      m.id,
      m.chat_id,
      m.sender_id,
      m.text,
      m.timestamp,
      m.local_id);
  return m;
}

inline crow::json::wvalue to_crow_json(const Message& m) {
  crow::json::wvalue j;
  LOG_INFO(
      "[Message] id '{}' | chat_id '{}' | sender_id '{}' | text '{}' | "
      "timestamp '{}' | local_id '{}'",
      m.id,
      m.chat_id,
      m.sender_id,
      m.text,
      m.timestamp,
      m.local_id);
  j["id"]        = m.id;
  j["chat_id"]   = m.chat_id;
  j["sender_id"] = m.sender_id;
  j["text"]      = m.text;
  j["timestamp"] = QDateTime::fromSecsSinceEpoch(m.timestamp).toString(Qt::ISODate).toStdString();
  j["local_id"]  = m.local_id;
  return j;
}

#endif  // BACKEND_NOTIFICATIONSERVICE_HEADERS_MESSAGE_H_
