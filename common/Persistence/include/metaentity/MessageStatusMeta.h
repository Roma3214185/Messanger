#ifndef METAENTITY_MESSAGES_STATUS_H
#define METAENTITY_MESSAGES_STATUS_H

#include "Meta.h"
#include "entities/MessageStatus.h"

template <> struct Reflection<MessageStatus> {
  static Meta meta() {
    return Meta{
        .table_name = MessageStatusTable::Table,
        .fields = {
            make_field<MessageStatus, long long>(MessageStatusTable::MessageId,
                                                 &MessageStatus::message_id),
            make_field<MessageStatus, long long>(MessageStatusTable::ReceiverId,
                                                 &MessageStatus::receiver_id),
            make_field<MessageStatus, bool>(MessageStatusTable::IsRead,
                                            &MessageStatus::is_read),
            make_field<MessageStatus, long long>(MessageStatusTable::ReatAt,
                                                 &MessageStatus::read_at)}};
  }
};

template <> struct Builder<MessageStatus> {
  static MessageStatus build(QSqlQuery &query) {
    MessageStatus message_status;
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

    assign(message_status.message_id);
    assign(message_status.receiver_id);
    assign(message_status.read_at);
    assign(message_status.is_read);

    return message_status;
  }
};

template <> struct EntityKey<MessageStatus> {
  static std::string get(const MessageStatus &entity) {
    return std::to_string(entity.message_id);
  }
};

inline constexpr auto kMessageStatusFields =
    std::make_tuple(&MessageStatus::message_id, &MessageStatus::receiver_id,
                    &MessageStatus::is_read, &MessageStatus::read_at);

template <> struct EntityFields<MessageStatus> {
  static constexpr auto &fields = kMessageStatusFields;
};

#endif // METAENTITY_MESSAGES_STATUS_H
