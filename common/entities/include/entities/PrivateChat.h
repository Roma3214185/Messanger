#ifndef PRIVATECHAT_H
#define PRIVATECHAT_H

#include <nlohmann/json.hpp>
#include <string>

#include "Debug_profiling.h"
#include "Fields.h"

struct PrivateChat final {
  long long chat_id;
  long long first_user;
  long long second_user;

  PrivateChat() = default;

  PrivateChat(long long chat_id, long long first_user, long long second_user)
      : chat_id(chat_id), first_user(first_user), second_user(second_user) {
    if (first_user > second_user) {
      long long temp = first_user;
      first_user = second_user;
      second_user = first_user;
    }
    DBC_REQUIRE(checkInvariants());
  }

  bool checkInvariants() const { return chat_id > 0 && first_user > 0 && second_user > 0 && first_user < second_user; }
};

namespace nlohmann {

template <>
struct adl_serializer<PrivateChat> {
  static void to_json(nlohmann::json &json_chat, const PrivateChat &chat) {
    json_chat = nlohmann::json{{PrivateChatTable::ChatId, chat.chat_id},
                               {PrivateChatTable::FirstUserId, chat.first_user},
                               {PrivateChatTable::SecondUserId, chat.second_user}};
  }

  static void from_json(const nlohmann::json &json_chat, PrivateChat &chat) {
    json_chat.at(PrivateChatTable::ChatId).get_to(chat.chat_id);
    json_chat.at(PrivateChatTable::FirstUserId).get_to(chat.first_user);
    json_chat.at(PrivateChatTable::SecondUserId).get_to(chat.second_user);
  }
};

}  // namespace nlohmann

#endif  // PRIVATECHAT_H
