#ifndef USERCHAT_H
#define USERCHAT_H

#include "include/entities/Chat.h"
#include "include/entities/ReactionInfo.h"

struct UserChat {
  Chat chat_info;
  std::vector<ReactionInfo> default_reactions;
};

namespace nlohmann {

template <>
struct adl_serializer<UserChat> {
    static void to_json(nlohmann::json &json_chat, const UserChat &chat) {
      json_chat = nlohmann::json(chat.chat_info);
      json_chat["default_reactions"] = chat.default_reactions;
    }

    static void from_json(const nlohmann::json &json_chat, UserChat &chat) {
      json_chat = nlohmann::json::parse(chat.chat_info);
      json_chat.at("default_reactions").get_to(chat.default_reactions;
    }
};

}  // namespace nlohmann

#endif // USERCHAT_H
