#ifndef PRIVATECHAT_H
#define PRIVATECHAT_H

#include <QDateTime>
#include <nlohmann/json.hpp>
#include <string>

#include "Meta.h"
#include "Fields.h"

struct PrivateChat {
    long long   chat_id;
    long long   first_user;
    long long   second_user;
};

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

#endif // PRIVATECHAT_H
