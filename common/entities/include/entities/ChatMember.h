#ifndef CHARMEMBER_H
#define CHARMEMBER_H

#include <nlohmann/json.hpp>
#include <string>

#include "Meta.h"
#include "Fields.h"

struct ChatMember {
  long long   chat_id;
  long long   user_id;
  std::string status;
  long long   added_at;
};

template <>
struct Reflection<ChatMember> {
  static Meta meta() {
    return Meta{.table_name = ChatMembersTable::Table,
                .fields     = {make_field<ChatMember, long long>(ChatMembersTable::ChatId, &ChatMember::chat_id),
                               make_field<ChatMember, long long>(ChatMembersTable::UserId, &ChatMember::user_id),
                               make_field<ChatMember, std::string>(ChatMembersTable::Status, &ChatMember::status),
                               make_field<ChatMember, long long>(ChatMembersTable::AddedAt, &ChatMember::added_at)}};
  }
};

template <>
struct Builder<ChatMember> {
  static ChatMember build(QSqlQuery& query) {
    ChatMember chat_member;
    int        idx = 0;

    auto assign = [&](auto& field) -> void {
      using TField         = std::decay_t<decltype(field)>;
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

    assign(chat_member.chat_id);
    assign(chat_member.user_id);
    assign(chat_member.status);
    assign(chat_member.added_at);

    return chat_member;
  }
};

inline constexpr auto ChatMemberFields = std::make_tuple(
    &ChatMember::chat_id, &ChatMember::user_id, &ChatMember::status, &ChatMember::added_at);

template <>
struct EntityFields<ChatMember> {
  static constexpr auto& fields = ChatMemberFields;
};

template <>
struct EntityKey<ChatMember> {
  static std::string get(const ChatMember& entity) {
    return std::to_string(entity.chat_id) + ", " + std::to_string(entity.user_id);
  }
};

namespace nlohmann {

template <>
struct adl_serializer<ChatMember> {
  static void to_json(nlohmann::json& json_chat_member, const ChatMember& chat_member) {
    json_chat_member = nlohmann::json{{ChatMembersTable::ChatId, chat_member.chat_id},
                                      {ChatMembersTable::UserId, chat_member.user_id},
                                      {ChatMembersTable::Status, chat_member.status},
                                      {ChatMembersTable::AddedAt, chat_member.added_at}};
  }

  static void from_json(const nlohmann::json& json_chat_member, ChatMember& chat_member) {
    json_chat_member.at(ChatMembersTable::ChatId).get_to(chat_member.chat_id);
    json_chat_member.at(ChatMembersTable::UserId).get_to(chat_member.user_id);
    json_chat_member.at(ChatMembersTable::Status).get_to(chat_member.status);
    json_chat_member.at(ChatMembersTable::AddedAt).get_to(chat_member.added_at);
  }
};

}  // namespace nlohmann

#endif  // CHARMEMBER_H
