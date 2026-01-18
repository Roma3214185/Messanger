#ifndef BACKEND_MESSAGESERVICE_HEADERS_MESSAGE_H_
#define BACKEND_MESSAGESERVICE_HEADERS_MESSAGE_H_

#include <crow.h>  //TODO: remove crow from here

#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <tuple>

#include "Debug_profiling.h"
#include "Fields.h"
#include "TimestampService.h"

struct Message final {
  long long id{0};
  long long chat_id{0};
  long long sender_id{0};
  long long timestamp{0};
  std::string text;
  std::string local_id;
  std::optional<long long> answer_on;
  // todo: bool is_resented or std::optional<long long> resented_from

  Message() = default;

  Message(long long p_id, long long p_chat_id, long long p_sender_id, long long p_timestamp, std::string p_text,
          std::string p_local_id, std::optional<long long> p_answer_on = std::nullopt)
      : id(p_id),
        chat_id(p_chat_id),
        sender_id(p_sender_id),
        timestamp(p_timestamp),
        text(std::move(p_text)),
        local_id(std::move(p_local_id)),
        answer_on(p_answer_on) {
    DBC_REQUIRE(checkInvariants());
  }

  bool checkInvariants() const {
    return id > 0 && chat_id > 0 && sender_id > 0 && !text.empty() && timestamp != 0 && !local_id.empty() &&
           (!answer_on.has_value() || answer_on.value() > 0);
  }
};

namespace nlohmann {

template <>
struct adl_serializer<Message> {
  static void to_json(nlohmann::json &json_message, const Message &message) {
    json_message = nlohmann::json{{MessageTable::Id, message.id},
                                  {MessageTable::ChatId, message.chat_id},
                                  {MessageTable::SenderId, message.sender_id},
                                  {MessageTable::Text, message.text},
                                  {MessageTable::Timestamp, message.timestamp},
                                  {MessageTable::LocalId, message.local_id}};

    if (message.answer_on.has_value()) {
      json_message[MessageTable::AnswerOn] = *message.answer_on;
    }
  }

  static void from_json(const nlohmann::json &json_message, Message &message) {
    json_message.at(MessageTable::Id).get_to(message.id);
    json_message.at(MessageTable::ChatId).get_to(message.chat_id);
    json_message.at(MessageTable::SenderId).get_to(message.sender_id);
    json_message.at(MessageTable::Text).get_to(message.text);
    json_message.at(MessageTable::Timestamp).get_to(message.timestamp);
    json_message.at(MessageTable::LocalId).get_to(message.local_id);

    if (json_message.contains(MessageTable::AnswerOn)) {
      message.answer_on = json_message.at(MessageTable::AnswerOn).get<long long>();
    } else {
      message.answer_on.reset();
    }
  }
};

}  // namespace nlohmann

namespace utils::entities {

inline Message from_crow_json(const crow::json::rvalue &json_message) {
  Message message;

  message.id = json_message.count(MessageTable::Id) ? json_message[MessageTable::Id].i() : 0;
  message.chat_id = json_message[MessageTable::ChatId].i();
  message.sender_id = json_message[MessageTable::SenderId].i();
  message.text = json_message[MessageTable::Text].s();
  message.local_id = json_message[MessageTable::LocalId].s();

  LOG_INFO("[Message] For text: {}, Local_id = ", message.text, message.local_id);

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

  if (json_message.has(MessageTable::AnswerOn)) {
    const auto &ans = json_message[MessageTable::AnswerOn];
    if (ans.t() == crow::json::type::Number) {
      message.answer_on = static_cast<long long>(ans.i());
    } else if (ans.t() == crow::json::type::Null) {
      message.answer_on.reset();
    } else {
      LOG_ERROR("Unexpected AnswerOn type");
      message.answer_on.reset();
    }
  } else {
    message.answer_on.reset();
  }

  LOG_INFO("From crow get message: {}", nlohmann::json(message).dump());

  return message;
}

inline crow::json::wvalue to_crow_json(const Message &message) {
  LOG_INFO("To crow set message: {}", nlohmann::json(message).dump());

  crow::json::wvalue json_message;
  json_message[MessageTable::Id] = message.id;
  json_message[MessageTable::ChatId] = message.chat_id;
  json_message[MessageTable::SenderId] = message.sender_id;
  json_message[MessageTable::Text] = message.text;
  json_message[MessageTable::Timestamp] = message.timestamp;
  json_message[MessageTable::LocalId] = message.local_id;

  if (message.answer_on.has_value()) {
    json_message[MessageTable::AnswerOn] = message.answer_on.value();
  }

  LOG_INFO("Local_id for text {} is {}", message.text, message.local_id);

  return json_message;
}

}  // namespace utils::entities

#endif  // BACKEND_MESSAGESERVICE_HEADERS_MESSAGE_H_
