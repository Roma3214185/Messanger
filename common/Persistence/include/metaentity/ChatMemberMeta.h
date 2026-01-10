#ifndef METAENTITY_CHATMEMBER_H
#define METAENTITY_CHATMEMBER_H

#include "Meta.h"
#include "entities/ChatMember.h"

template <>
struct Reflection<ChatMember> {
  static Meta meta() {
    return Meta{.table_name = ChatMemberTable::Table,
                .fields = {make_field<ChatMember, long long>(ChatMemberTable::ChatId, &ChatMember::chat_id),
                           make_field<ChatMember, long long>(ChatMemberTable::UserId, &ChatMember::user_id),
                           make_field<ChatMember, std::string>(ChatMemberTable::Status, &ChatMember::status),
                           make_field<ChatMember, long long>(ChatMemberTable::AddedAt, &ChatMember::added_at)}};
  }
};

template <>
struct Builder<ChatMember> {
  static ChatMember build(const QSqlQuery &query) {
    ChatMember chat_member;
    int idx = 0;

    auto assign = [&](auto &field) -> void {
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

    assign(chat_member.chat_id);
    assign(chat_member.user_id);
    assign(chat_member.status);
    assign(chat_member.added_at);

    return chat_member;
  }
};

inline constexpr auto ChatMemberFields =
    std::make_tuple(&ChatMember::chat_id, &ChatMember::user_id, &ChatMember::status, &ChatMember::added_at);

template <>
struct EntityFields<ChatMember> {
  static constexpr auto &fields = ChatMemberFields;
};

template <>
struct EntityKey<ChatMember> {
  static std::string get(const ChatMember &entity) {
    return std::to_string(entity.chat_id) + ", " + std::to_string(entity.user_id);
  }
};

#endif  // METAENTITY_CHATMEMBER_H
