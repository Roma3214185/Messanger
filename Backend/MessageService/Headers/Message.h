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
    QDateTime timestamp;
};

struct MessageStatus {
    long long id;
    long long receiver_id;
    bool is_read = false;
    QDateTime read_at = QDateTime();
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
                make_field<Message, QDateTime>("timestamp", &Message::timestamp)
            }
        };
    }
};

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
                make_field<MessageStatus, QDateTime>("read_at", &MessageStatus::read_at)
            }
        };
    }
};


inline void to_json(json& j, const Message& m) {
    j = json{
        {"id", m.id},
        {"chat_id", m.chat_id},
        {"sender_id", m.sender_id},
        {"text", m.text},
        {"timestamp", m.timestamp.toString(Qt::ISODate).toStdString()} // ✅ serialize as string
    };
}

inline void from_json(const json& j, Message& u) {
    j.at("id").get_to(u.id);
    j.at("chat_id").get_to(u.chat_id);
    j.at("sender_id").get_to(u.sender_id);
    j.at("text").get_to(u.text);

    std::string ts;
    j.at("timestamp").get_to(ts);
    u.timestamp = QDateTime::fromString(QString::fromStdString(ts), Qt::ISODate); // ✅ parse back
}

inline void to_json(json& j, const MessageStatus& m) {
    j = json{
        {"id", m.id},
        {"receiver_id", m.receiver_id},
        {"is_read", m.is_read},
        {"read_at", m.read_at.toString(Qt::ISODate).toStdString()} // ✅ serialize as string
    };
}

inline void from_json(const json& j, MessageStatus& u) {
    j.at("id").get_to(u.id);
    j.at("receiver_id").get_to(u.receiver_id);
    j.at("is_read").get_to(u.is_read);

    std::string ts;
    j.at("read_at").get_to(ts);
    u.read_at = QDateTime::fromString(QString::fromStdString(ts), Qt::ISODate); // ✅ parse back
}



#endif // MESSAGE_H



