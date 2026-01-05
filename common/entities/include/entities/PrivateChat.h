#ifndef PRIVATECHAT_H
#define PRIVATECHAT_H

#include <nlohmann/json.hpp>
#include <string>

#include "Fields.h"
#include "interfaces/entity.h"

struct PrivateChat final : public IEntity {
  long long chat_id;
  long long first_user;
  long long second_user;
};

namespace nlohmann {

template <>
struct adl_serializer<PrivateChat> {
  static void to_json(nlohmann::json& json_chat, const PrivateChat& chat) {
    json_chat = nlohmann::json{{PrivateChatTable::ChatId, chat.chat_id},
                               {PrivateChatTable::FirstUserId, chat.first_user},
                               {PrivateChatTable::SecondUserId, chat.second_user}};
  }

  static void from_json(const nlohmann::json& json_chat, PrivateChat& chat) {
    json_chat.at(PrivateChatTable::ChatId).get_to(chat.chat_id);
    json_chat.at(PrivateChatTable::FirstUserId).get_to(chat.first_user);
    json_chat.at(PrivateChatTable::SecondUserId).get_to(chat.second_user);
  }
};

}  // namespace nlohmann

#endif  // PRIVATECHAT_H
