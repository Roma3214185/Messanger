#ifndef BACKEND_MESSAGESERVICE_HEADERS_MESSAGE_H_
#define BACKEND_MESSAGESERVICE_HEADERS_MESSAGE_H_

#include <crow.h> //TODO: remove crow from here

#include <nlohmann/json.hpp>
#include <string>
#include <tuple>

#include "Debug_profiling.h"
#include "Fields.h"
#include "TimestampService.h"
#include "interfaces/entity.h"

struct Message final {
  long long id{0};
  long long chat_id{0};
  long long sender_id{0};
  long long timestamp{0};
  std::string text;
  std::string local_id;

  Message() = default;

  Message(long long id, long long chat_id,
          long long sender_id, long  long timestamp,
          std::string text, std::string local_id)
      : id(id), chat_id(chat_id), sender_id(sender_id), timestamp(timestamp),
      text(std::move(text)), local_id(std::move(local_id))
  {
    DBC_REQUIRE(checkInvariants());
  }

  bool checkInvariants() const {
    return id > 0
        && chat_id > 0
        && sender_id > 0
        && !text.empty();
  }
};

namespace utils::entities {

inline Message from_crow_json(const crow::json::rvalue &json_message) {
  Message message;

  message.id = json_message.count(MessageTable::Id)
                   ? json_message[MessageTable::Id].i()
                   : 0;
  message.chat_id = json_message[MessageTable::ChatId].i();
  message.sender_id = json_message[MessageTable::SenderId].i();
  message.text = json_message[MessageTable::Text].s();
  message.local_id = json_message[MessageTable::LocalId].s();

  LOG_INFO("[Message] For text: {}, Local_id = ", message.text,
           message.local_id);

  if (json_message.count(MessageTable::Timestamp)) {
    const auto &ts_val = json_message[MessageTable::Timestamp];
    if (ts_val.t() == crow::json::type::String) {
      message.timestamp = TimestampService::parseTimestampISO8601(ts_val.s());
    } else if (ts_val.t() == crow::json::type::Number) {
      message.timestamp = static_cast<long long>(ts_val.i());
    } else {
      LOG_ERROR("Unexpected timestamp type");
    }
  }

  return message;
}

inline crow::json::wvalue to_crow_json(const Message &message) {
  crow::json::wvalue json_message;
  json_message[MessageTable::Id] = message.id;
  json_message[MessageTable::ChatId] = message.chat_id;
  json_message[MessageTable::SenderId] = message.sender_id;
  json_message[MessageTable::Text] = message.text;
  json_message[MessageTable::Timestamp] = message.timestamp;
  json_message[MessageTable::LocalId] = message.local_id;

  LOG_INFO("Local_id for text {} is {}", message.text, message.local_id);

  return json_message;
}

} // namespace utils::entities

namespace nlohmann {

template <> struct adl_serializer<Message> {
  static void to_json(nlohmann::json &json_message, const Message &message) {
    json_message = nlohmann::json{{MessageTable::Id, message.id},
                                  {MessageTable::ChatId, message.chat_id},
                                  {MessageTable::SenderId, message.sender_id},
                                  {MessageTable::Text, message.text},
                                  {MessageTable::Timestamp, message.timestamp},
                                  {MessageTable::LocalId, message.local_id}};
  }

  static void from_json(const nlohmann::json &json_message, Message &message) {
    json_message.at(MessageTable::Id).get_to(message.id);
    json_message.at(MessageTable::ChatId).get_to(message.chat_id);
    json_message.at(MessageTable::SenderId).get_to(message.sender_id);
    json_message.at(MessageTable::Text).get_to(message.text);
    json_message.at(MessageTable::Timestamp).get_to(message.timestamp);
    json_message.at(MessageTable::LocalId).get_to(message.local_id);
  }
};

} // namespace nlohmann

#endif // BACKEND_MESSAGESERVICE_HEADERS_MESSAGE_H_
