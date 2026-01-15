#ifndef USERMESSAGE_H
#define USERMESSAGE_H

#include <optional>

#include "Message.h"
#include "ReactionInfo.h"

struct UserMessage {  // todo: rename UserMessage -> Message
  Message message;    // todo: rename Message -> MessageInfo

  struct {
    int count;
    bool read_by_me;
  } read;

  struct {
    std::unordered_map<ReactionInfo, int> counts;  // type -> count
    std::optional<long long> my_reaction;
  } reactions;
};

namespace nlohmann {

template <>
struct adl_serializer<UserMessage> {
  static void to_json(nlohmann::json &j, const UserMessage &m) {
    j = nlohmann::json(m.message);
    j["read"] = {{"count", m.read.count}, {"receiver_read_status", m.read.read_by_me}};

    j["reactions"]["counts"] = m.reactions.counts;

    if (m.reactions.my_reaction.has_value())
      j["reactions"]["receiver_reaction"] = *m.reactions.my_reaction;
    else
      j["reactions"]["receiver_reaction"] = nullptr;
  }

  static void from_json(const nlohmann::json &j, UserMessage &m) {
    j.at("message").get_to(m.message);

    if (j.contains("read")) {
      if (j["read"].contains("receiver_read_status")) j["read"].at("receiver_read_status").get_to(m.read.read_by_me);
      if (j["read"].contains("count")) j["read"].at("count").get_to(m.read.count);
    }

    if (j.contains("reactions")) {
      if (j["reactions"].contains("counts")) j["reactions"].at("counts").get_to(m.reactions.counts);
      if (j.contains("reactions") && j["reactions"].contains("receiver_reaction") &&
          !j["reactions"]["receiver_reaction"].is_null()) {
        m.reactions.my_reaction = j["reactions"]["receiver_reaction"].get<int>();
      } else {
        m.reactions.my_reaction.reset();
      }
    }
  }
};

}  // namespace nlohmann

#endif  // USERMESSAGE_H
