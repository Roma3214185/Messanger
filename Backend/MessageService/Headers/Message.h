#ifndef MESSAGE_H
#define MESSAGE_H

#include <string>
#include <QString>
#include <functional>
#include <any>
#include <QDateTime>
#include "../../GenericRepository/GenericReposiroty.h"
#include "../../RedisCashe/RedisCache.h"

struct Message{
    long long id;
    long long chat_id;
    long long sender_id;
    std::string text;
    long long timestamp;
};

struct MessageStatus {
    long long id;
    long long receiver_id;
    bool is_read = false;
    long long read_at = QDateTime::currentDateTime().toSecsSinceEpoch();
};

template <>
struct Reflection<Message>{
    static Meta meta(){
        return Meta{
            .name = "Messages",
            .tableName = "messages",
            .fields = {
                make_field<Message, long long>("id", &Message::id),
                make_field<Message, long long>("sender_id", &Message::sender_id),
                make_field<Message, long long>("chat_id", &Message::chat_id),
                make_field<Message, std::string>("text", &Message::text),
                make_field<Message, long long>("timestamp", &Message::timestamp)
            }
        };
    }
};

// template<typename T>
// struct Builder;

// #define DEFINE_BUILDER(Entity, ...)                       \
// template<>                                               \
//     struct Builder<Entity> {                                 \
//         static Entity build(QSqlQuery& query) {             \
//             Entity e;                                       \
//             int i = 0;                                      \
//             ((__VA_ARGS__ = query.value(i++).toVariant()), ...); \
//             return e;                                       \
//     }                                                   \
// };

// Генерація Builder<Message>
//DEFINE_BUILDER(Message, id, chat_id, sender_id, text, timestamp)


// template<>
// struct Builder<Message> {
//     Message build(QSqlQuery& query) const {
//         Message m;
//         m.id        = query.value("id").toInt();
//         m.chat_id   = query.value("chat_id").toInt();
//         m.sender_id = query.value("sender_id").toInt();
//         m.text      = query.value("text").toString();
//         m.timestamp = query.value("timestamp").toInt();
//         return m;
//     }
// };

template <>
struct Reflection<MessageStatus>{
    static Meta meta(){
        return Meta{
            .name = "MessageStatus",
            .tableName = "messages_status",
            .fields = {
                make_field<MessageStatus, long long>("id", &MessageStatus::id),
                make_field<MessageStatus, long long>("receiver_id", &MessageStatus::receiver_id),
                make_field<MessageStatus, bool>("is_read", &MessageStatus::is_read),
                make_field<MessageStatus, long long>("read_at", &MessageStatus::read_at)
            }
        };
    }
};

template<>
struct Builder<Message> {
    static Message build(QSqlQuery& query) {
        Message e;
        int i = 0;

        auto assign = [&](auto& field) {
            using TField = std::decay_t<decltype(field)>;
            QVariant value = query.value(i++);
            if constexpr (std::is_same_v<TField, long long>)
                field = value.toLongLong();
            else if constexpr (std::is_same_v<TField, int>)
                field = value.toInt();
            else if constexpr (std::is_same_v<TField, std::string>)
                field = value.toString().toStdString();
            else if constexpr (std::is_same_v<TField, QString>)
                field = value.toString();
            else
                field = value.value<TField>();
        };

        // assign(e.id);
        // assign(e.chat_id);
        // assign(e.sender_id);
        // assign(e.text);
        // assign(e.timestamp);

        return e;
    }
};

template<>
struct Builder<MessageStatus> {
    static MessageStatus build(QSqlQuery& query) {
        MessageStatus e;
        int i = 0;

        auto assign = [&](auto& field) {
            using TField = std::decay_t<decltype(field)>;
            QVariant value = query.value(i++);
            if constexpr (std::is_same_v<TField, long long>)
                field = value.toLongLong();
            else if constexpr (std::is_same_v<TField, int>)
                field = value.toInt();
            else if constexpr (std::is_same_v<TField, std::string>)
                field = value.toString().toStdString();
            else if constexpr (std::is_same_v<TField, QString>)
                field = value.toString();
            else
                field = value.value<TField>();
        };

        // assign(e.id);
        // assign(e.chat_id);
        // assign(e.sender_id);
        // assign(e.text);
        // assign(e.timestamp);

        return e;
    }
};

inline constexpr auto MessageFields = std::make_tuple(
    &Message::id,
    &Message::chat_id,
    &Message::sender_id,
    &Message::text,
    &Message::timestamp
    );

inline constexpr auto MessageStatusFields = std::make_tuple(
    &MessageStatus::id,
    &MessageStatus::receiver_id,
    &MessageStatus::is_read,
    &MessageStatus::read_at
    );


// inline constexpr std::array<MessageFields, 4> uFields = {{
//     {"id", typeid(long long)},
//     {"chat_id", typeid(long long)},
//     {"sender_id", typeid(long long)},
//     {"text", typeid(QDateTime)},
//     {"timestamp", typeid(long long)}

// }};

template<>
struct EntityFields<Message> {
    static constexpr auto& fields = MessageFields;
};

template<>
struct EntityFields<MessageStatus> {
    static constexpr auto& fields = MessageStatusFields;
};



inline void to_json(json& j, const Message& m) {
    j = json{
        {"id", m.id},
        {"chat_id", m.chat_id},
        {"sender_id", m.sender_id},
        {"text", m.text},
        {"timestamp", m.timestamp}
    };
}

inline void from_json(const json& j, Message& u) {
    j.at("id").get_to(u.id);
    j.at("chat_id").get_to(u.chat_id);
    j.at("sender_id").get_to(u.sender_id);
    j.at("text").get_to(u.text);
    j.at("timestamp").get_to(u.timestamp);
}

inline void to_json(json& j, const MessageStatus& m) {
    j = json{
        {"id", m.id},
        {"receiver_id", m.receiver_id},
        {"is_read", m.is_read},
        {"read_at", m.read_at}
    };
}

inline void from_json(const json& j, MessageStatus& u) {
    j.at("id").get_to(u.id);
    j.at("receiver_id").get_to(u.receiver_id);
    j.at("is_read").get_to(u.is_read);
    j.at("read_at").get_to(u.read_at);
}





#endif // MESSAGE_H



