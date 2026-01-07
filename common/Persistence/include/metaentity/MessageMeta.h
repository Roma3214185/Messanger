#ifndef MESSAGEMETA_H
#define MESSAGEMETA_H

#include "Meta.h"
#include "entities/Message.h"

template <> struct Reflection<Message> {
  static Meta meta() {
    return Meta{.table_name = MessageTable::Table,
                .fields = {make_field<Message, long long>(MessageTable::Id,
                                                          &Message::id),
                           make_field<Message, long long>(
                               MessageTable::SenderId, &Message::sender_id),
                           make_field<Message, long long>(MessageTable::ChatId,
                                                          &Message::chat_id),
                           make_field<Message, std::string>(MessageTable::Text,
                                                            &Message::text),
                           make_field<Message, long long>(
                               MessageTable::Timestamp, &Message::timestamp),
                           make_field<Message, std::string>(
                               MessageTable::LocalId, &Message::local_id)}};
  }
};

template <> struct Builder<Message> {
  static Message build(QSqlQuery &query) {
    Message message;
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

    assign(message.id);
    assign(message.chat_id);
    assign(message.sender_id);
    assign(message.timestamp);
    assign(message.text);
    assign(message.local_id);

    return message;
  }
};

inline constexpr auto MessageFields =
    std::make_tuple(&Message::id, &Message::chat_id, &Message::sender_id,
                    &Message::text, &Message::timestamp);

template <> struct EntityFields<Message> {
  static constexpr auto &fields = MessageFields;
};

#endif // MESSAGEMETA_H
