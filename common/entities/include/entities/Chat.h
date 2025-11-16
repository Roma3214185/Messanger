#ifndef BACKEND_CHATSERVICE_SRC_HEADERS_CHAT_H_
#define BACKEND_CHATSERVICE_SRC_HEADERS_CHAT_H_

#include <QDateTime>
#include <nlohmann/json.hpp>
#include <string>

#include "Meta.h"
#include "Fields.h"

struct Chat {
  long long   id       = 0;
  int         is_group = 0;
  std::string name;
  std::string avatar;
  long long   created_at;
};

template <>
struct Reflection<Chat> {
  static Meta meta() {
    return Meta{.table_name = ChatTable::Table,
                .fields     = {make_field<Chat, long long>(ChatTable::Id, &Chat::id),
                               make_field<Chat, int>(ChatTable::IsGroup, &Chat::is_group),
                               make_field<Chat, std::string>(ChatTable::Name, &Chat::name),
                               make_field<Chat, std::string>(ChatTable::Avatar, &Chat::avatar),
                               make_field<Chat, long long>(ChatTable::CreatedAt, &Chat::created_at)}};
  }
};

template <>
struct Builder<Chat> {
  static Chat build(QSqlQuery& query) {
    Chat chat;
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
  static constexpr auto& fields = ChatFields;
};

namespace nlohmann {

template <>
struct adl_serializer<Chat> {
  static void to_json(nlohmann::json& json_chat, const Chat& chat) {
    json_chat = nlohmann::json{{ChatTable::Id, chat.id},
                               {ChatTable::IsGroup, chat.is_group},
                               {ChatTable::Name, chat.name},
                               {ChatTable::Avatar, chat.avatar},
                               {ChatTable::CreatedAt, chat.created_at}};
  }

  static void from_json(const nlohmann::json& json_chat, Chat& chat) {
    json_chat.at(ChatTable::Id).get_to(chat.id);
    json_chat.at(ChatTable::IsGroup).get_to(chat.is_group);
    json_chat.at(ChatTable::Name).get_to(chat.name);
    json_chat.at(ChatTable::Avatar).get_to(chat.avatar);
    json_chat.at(ChatTable::CreatedAt).get_to(chat.created_at);
  }
};

}  // namespace nlohmann

#endif  // BACKEND_CHATSERVICE_SRC_HEADERS_CHAT_H_
