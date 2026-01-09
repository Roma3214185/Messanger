#ifndef USERMESSAGE_H
#define USERMESSAGE_H

#include <optional>

#include "Message.h"

struct Reaction final {
  long long message_id;
  long long user_id;
  int type;  // 1 -> like, 2 -> smile, 3 -> dislike, 4 -> heart

  Reaction(long long message_id, long long user_id, int type) : message_id(message_id), user_id(user_id), type(type) {
    DBC_REQUIRE(checkInvariants());
  }

  bool checkInvariants() const { return message_id > 0 && user_id > 0 && type > 0; }
};

struct UserMessage {  // todo: rename UserMessage -> Message
  Message message;    // todo: rename Message -> MessageInfo

  struct {
    int count;
    bool read_by_me;
  } read;

  struct {
    std::unordered_map<int, int> counts;  // type -> count
    std::optional<int> my_reaction;
  } reactions;
};

namespace nlohmann {

template <>
struct adl_serializer<Reaction> {
  static void to_json(nlohmann::json &json_message, const Reaction &user_reaction) {
    json_message["message_id"] = user_reaction.message_id;
    json_message["user_id"] = user_reaction.user_id;
    json_message["type"] = user_reaction.type;
  }

  static void from_json(const nlohmann::json &json_message, Reaction &user_reaction) {
    user_reaction.message_id = json_message["message_id"];
    user_reaction.user_id = json_message["user_id"];
    user_reaction.type = json_message["type"];
  }
};

}  // namespace nlohmann

namespace nlohmann {

template <>
struct adl_serializer<UserMessage> {
  static void to_json(nlohmann::json &j, const UserMessage &m) {
    j = nlohmann::json(m.message);
    j["read"] = {{"count", m.read.count}, {"read_by_me", m.read.read_by_me}};

    j["reactions"]["counts"] = m.reactions.counts;

    if (m.reactions.my_reaction.has_value())
      j["reactions"]["my_reaction"] = *m.reactions.my_reaction;
    else
      j["reactions"]["my_reaction"] = nullptr;
  }

  static void from_json(const nlohmann::json &j, UserMessage &m) {
    j.at("message").get_to(m.message);

    if (j.contains("read")) {
      if (j["read"].contains("read_by_me")) j["read"].at("read_by_me").get_to(m.read.read_by_me);
      if (j["read"].contains("count")) j["read"].at("count").get_to(m.read.count);
    }

    if (j.contains("reactions")) {
      if (j["reactions"].contains("counts")) j["reactions"].at("counts").get_to(m.reactions.counts);
      if (j.contains("reactions") && j["reactions"].contains("my_reaction") &&
          !j["reactions"]["my_reaction"].is_null()) {
        m.reactions.my_reaction = j["reactions"]["my_reaction"].get<int>();
      } else {
        m.reactions.my_reaction.reset();
      }
    }
  }
};

}  // namespace nlohmann

#endif  // USERMESSAGE_H
