#ifndef CHARMEMBER_H
#define CHARMEMBER_H

#include <nlohmann/json.hpp>
#include <string>

#include "Fields.h"
#include "interfaces/entity.h"

struct ChatMember final : public IEntity {
    long long   chat_id{ 0 };
    long long   user_id{ 0 };
    std::string status;
    long long   added_at { 0 };
};

namespace nlohmann {

template <>
struct adl_serializer<ChatMember> {
  static void to_json(nlohmann::json& json_chat_member, const ChatMember& chat_member) {
    json_chat_member = nlohmann::json{{ChatMemberTable::ChatId, chat_member.chat_id},
                                      {ChatMemberTable::UserId, chat_member.user_id},
                                      {ChatMemberTable::Status, chat_member.status},
                                      {ChatMemberTable::AddedAt, chat_member.added_at}};
  }

  static void from_json(const nlohmann::json& json_chat_member, ChatMember& chat_member) {
    json_chat_member.at(ChatMemberTable::ChatId).get_to(chat_member.chat_id);
    json_chat_member.at(ChatMemberTable::UserId).get_to(chat_member.user_id);
    json_chat_member.at(ChatMemberTable::Status).get_to(chat_member.status);
    json_chat_member.at(ChatMemberTable::AddedAt).get_to(chat_member.added_at);
  }
};

}  // namespace nlohmann

#endif  // CHARMEMBER_H
