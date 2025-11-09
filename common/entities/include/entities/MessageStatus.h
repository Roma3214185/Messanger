#ifndef BACKEND_MESSAGESERVICE_HEADERS_MESSAGESTATUS_H_
#define BACKEND_MESSAGESERVICE_HEADERS_MESSAGESTATUS_H_

#include <QDateTime>
#include <QString>
#include <cstdint>
#include <nlohmann/json.hpp>
#include <string>
#include <tuple>

#include "Meta.h"

struct MessageStatus {
  long long message_id;
  long long receiver_id;
  long long read_at = QDateTime::currentSecsSinceEpoch();
  bool is_read = false;
};

template <>
struct Reflection<MessageStatus> {
  static Meta meta() {
    return Meta{
        .name = "MessageStatus",
        .table_name = "messages_status",
        .fields = {
            make_field<MessageStatus, long long>("message_id", &MessageStatus::message_id),
            make_field<MessageStatus, long long>("receiver_id",
                                               &MessageStatus::receiver_id),
            make_field<MessageStatus, bool>("is_read", &MessageStatus::is_read),
            make_field<MessageStatus, long long>("read_at",
                                               &MessageStatus::read_at)}};
  }
};

template <>
struct Builder<MessageStatus> {
  static MessageStatus build(QSqlQuery& query) {
    MessageStatus message_status;
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

    assign(message_status.message_id);
    assign(message_status.receiver_id);
    assign(message_status.read_at);
    assign(message_status.is_read);

    return message_status;
  }
};

template <>
struct EntityKey<MessageStatus> {
    static std::string get(const MessageStatus& entity) {
      return std::to_string(entity.message_id);
    }
};

inline constexpr auto kMessageStatusFields =
    std::make_tuple(&MessageStatus::message_id, &MessageStatus::receiver_id,
                    &MessageStatus::is_read, &MessageStatus::read_at);

template <>
struct EntityFields<MessageStatus> {
  static constexpr auto& fields = kMessageStatusFields;
};

namespace nlohmann {

template <>
struct adl_serializer<MessageStatus> {
  static void to_json(nlohmann::json& json_message_status,
                      const MessageStatus& message_status) {
    json_message_status =
        nlohmann::json{{"message_id", message_status.message_id},
                       {"receiver_id", message_status.receiver_id},
                       {"is_read", message_status.is_read},
                       {"read_at", message_status.read_at}};
  }

  static void from_json(const nlohmann::json& json_message_status,
                        MessageStatus& message_status) {
    json_message_status.at("message_id").get_to(message_status.message_id);
    json_message_status.at("receiver_id").get_to(message_status.receiver_id);
    json_message_status.at("is_read").get_to(message_status.is_read);
    json_message_status.at("read_at").get_to(message_status.read_at);
  }
};

}  //nlohmann

#endif  // BACKEND_MESSAGESERVICE_HEADERS_MESSAGESTATUS_H_
