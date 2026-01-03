#ifndef BACKEND_MESSAGESERVICE_HEADERS_MESSAGESTATUS_H_
#define BACKEND_MESSAGESERVICE_HEADERS_MESSAGESTATUS_H_

#include <cstdint>
#include <nlohmann/json.hpp>
#include <string>
#include <tuple>

#include "Fields.h"
#include "interfaces/entity.h"

inline long long getCurrentTime() {
  using namespace std::chrono;
  return duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
}

struct MessageStatus final : public IEntity {
  long long message_id{ 0 };
  long long receiver_id{ 0 };
  long long read_at{ 0 };
  bool      is_read{ false };
};

namespace nlohmann {

template <>
struct adl_serializer<MessageStatus> {
  static void to_json(nlohmann::json& json_message_status, const MessageStatus& message_status) {
    json_message_status = nlohmann::json{{MessageStatusTable::MessageId, message_status.message_id},
                                         {MessageStatusTable::ReceiverId, message_status.receiver_id},
                                         {MessageStatusTable::IsRead, message_status.is_read},
                                         {MessageStatusTable::ReatAt, message_status.read_at}};
  }

  static void from_json(const nlohmann::json& json_message_status, MessageStatus& message_status) {
    json_message_status.at(MessageStatusTable::MessageId).get_to(message_status.message_id);
    json_message_status.at(MessageStatusTable::ReceiverId).get_to(message_status.receiver_id);
    json_message_status.at(MessageStatusTable::IsRead).get_to(message_status.is_read);
    json_message_status.at(MessageStatusTable::ReatAt).get_to(message_status.read_at);
  }
};

}  // namespace nlohmann

#endif  // BACKEND_MESSAGESERVICE_HEADERS_MESSAGESTATUS_H_
