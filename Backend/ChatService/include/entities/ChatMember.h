#ifndef CHARMEMBER_H
#define CHARMEMBER_H

#include <string>

#include <nlohmann/json.hpp>

#include "Meta.h"

struct ChatMember {
    long long id;
    long long user_id;
    std::string status;
    long long added_at;
};

template <>
struct Reflection<ChatMember> {
    static Meta meta() {
      return Meta{
          .name = "chat_members",
          .table_name = "chat_members",
          .fields = {
                     make_field<ChatMember, long long>("id", &ChatMember::id),
                     make_field<ChatMember, long long>("user_id", &ChatMember::user_id),
                     make_field<ChatMember, std::string>("status", &ChatMember::status),
                     make_field<ChatMember, long long>("added_at", &ChatMember::added_at)}
      };
    }
};

template <>
struct Builder<ChatMember> {
    static ChatMember build(QSqlQuery& query) {
      ChatMember chat_member;
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

      assign(chat_member.id);
      assign(chat_member.user_id);
      assign(chat_member.status);
      assign(chat_member.added_at);

      return chat_member;
    }
};

inline constexpr auto ChatMemberFields =
    std::make_tuple(&ChatMember::id, &ChatMember::user_id, &ChatMember::status,
                    &ChatMember::added_at);

template <>
struct EntityFields<ChatMember> {
    static constexpr auto& fields = ChatMemberFields;
};

namespace nlohmann {

template <>
struct adl_serializer<ChatMember> {

  static void to_json(nlohmann::json& json_chat_member, const ChatMember& chat_member) {
    json_chat_member = nlohmann::json{{"id", chat_member.id},
                               {"user_id", chat_member.user_id},
                               {"status", chat_member.status},
                               {"added_at", chat_member.added_at}};
  }

  static void from_json(const nlohmann::json& json_chat_member, ChatMember& chat_member) {
    json_chat_member.at("id").get_to(chat_member.id);
    json_chat_member.at("user_id").get_to(chat_member.user_id);
    json_chat_member.at("status").get_to(chat_member.status);
    json_chat_member.at("added_at").get_to(chat_member.added_at);
  }
};

}   //nlohmann

#endif // CHARMEMBER_H
