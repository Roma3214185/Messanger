#ifndef METAENTITY_CHAT_H
#define METAENTITY_CHAT_H

#include "Meta.h"
#include "entities/Chat.h"

template <>
struct Reflection<Chat> {
  static Meta meta() {
    return Meta{.table_name = ChatTable::Table,
                .fields = {make_field<Chat, long long>(ChatTable::Id, &Chat::id),
                           make_field<Chat, int>(ChatTable::IsGroup, &Chat::is_group),
                           make_field<Chat, std::string>(ChatTable::Name, &Chat::name),
                           make_field<Chat, std::string>(ChatTable::Avatar, &Chat::avatar),
                           make_field<Chat, long long>(ChatTable::CreatedAt, &Chat::created_at)}};
  }
};

template <>
struct Builder<Chat> {
  static Chat build(QSqlQuery &query) {
    Chat chat;
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

    assign(chat.id);
    assign(chat.is_group);
    assign(chat.name);
    assign(chat.avatar);
    assign(chat.created_at);

    return chat;
  }
};

inline constexpr auto ChatFields =
    std::make_tuple(&Chat::id, &Chat::is_group, &Chat::name, &Chat::avatar, &Chat::created_at);

template <>
struct EntityFields<Chat> {
  static constexpr auto &fields = ChatFields;
};

#endif  // METAENTITY_CHAT_H
