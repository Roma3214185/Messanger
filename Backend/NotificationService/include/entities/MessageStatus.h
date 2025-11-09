#ifndef BACKEND_NOTIFICATIONSERVICE_HEADERS_MESSAGESTATUS_H_
#define BACKEND_NOTIFICATIONSERVICE_HEADERS_MESSAGESTATUS_H_

#include <QDateTime>
#include <nlohmann/json.hpp>

struct MessageStatus {
  long long message_id;
  long long receiver_id;
  bool      is_read = false;
  long long read_at;
};

namespace nlohmann {

template <>
struct adl_serializer<MessageStatus> {
  static void to_json(nlohmann::json& j, const MessageStatus& m) {
    j = nlohmann::json{{"message_id", m.message_id},
                       {"receiver_id", m.receiver_id},
                       {"is_read", m.is_read},
                       {"read_at", m.read_at}};
  }

  static void from_json(const nlohmann::json& j, MessageStatus& m) {
    j.at("message_id").get_to(m.message_id);
    j.at("receiver_id").get_to(m.receiver_id);
    j.at("is_read").get_to(m.is_read);
    j.at("read_at").get_to(m.read_at);
  }
};

}  // namespace nlohmann

#endif  // BACKEND_NOTIFICATIONSERVICE_HEADERS_MESSAGESTATUS_H_
