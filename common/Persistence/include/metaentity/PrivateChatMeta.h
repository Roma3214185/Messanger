#ifndef METAENTITY_PRIVATECHAT_H
#define METAENTITY_PRIVATECHAT_H

#include "Meta.h"
#include "entities/PrivateChat.h"

template <>
struct Reflection<PrivateChat> {
    static Meta meta() {
      return Meta{.table_name = PrivateChatTable::Table,
                  .fields     = {make_field<PrivateChat, long long>(PrivateChatTable::ChatId, &PrivateChat::chat_id),
                             make_field<PrivateChat, long long>(PrivateChatTable::FirstUserId, &PrivateChat::first_user),
                             make_field<PrivateChat, long long>(PrivateChatTable::SecondUserId, &PrivateChat::second_user)}};
    }
};

template <>
struct EntityKey<PrivateChat> {
    static std::string get(const PrivateChat& entity) {
      return std::to_string(entity.first_user) + ", " + std::to_string(entity.second_user);
    }
};


template <>
struct Builder<PrivateChat> {
    static PrivateChat build(QSqlQuery& query) {
      PrivateChat chat;
      int  idx = 0;

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

      assign(chat.chat_id);
      assign(chat.first_user);
      assign(chat.second_user);

      return chat;
    }
};

inline constexpr auto PrivateChatFields =
    std::make_tuple(&PrivateChat::chat_id, &PrivateChat::first_user, &PrivateChat::second_user);

template <>
struct EntityFields<PrivateChat> {
    static constexpr auto& fields = PrivateChatFields;
};

#endif // METAENTITY_PRIVATECHAT_H
