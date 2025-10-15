#ifndef MESSAGE_H
#define MESSAGE_H

#include <string>
#include <QString>
#include <functional>
#include <any>
#include <QDateTime>
#include <messagedatabase.h>
#include "../../GenericRepository/GenericReposiroty.h"
#include "../../RedisCashe/RedisCache.h"

struct Message{
    long long id;
    long long chat_id;
    long long sender_id;
    std::string text;
    long long receiver_id;
    QDateTime timestamp;
};

template <>
struct Reflection<Message>{
    static Meta meta(){
        return Meta{
            .name = "message",
            .tableName = "messages",
            .fields = {
                make_field<Message, long long>("id", &Message::id),
                make_field<Message, long long>("sender_id", &Message::sender_id),
                make_field<Message, long long>("chat_id", &Message::chat_id),
                make_field<Message, std::string>("text", &Message::text),
                make_field<Message, long long>("receiverId", &Message::receiver_id),
                make_field<Message, QDateTime>("timestamp", &Message::timestamp)
            }
        };
    }
};









// class MessageRepositry{
//     DataBase db;
//     MessageRepositry(DataBase& db) : db(db) {}

//     std::optional<Message> findOne(long long id){
//         Message msg;
//         auto meta = MessageReflection::meta();

//         QSqlQuery query(db.connection());
//         query.prepare("SELECT * FROM message WHERE id = ?");
//         db.executeQuery(query, id);

//         if (query.exec() && query.next()) {
//             for (const auto& field : meta.fields) {
//                 QVariant value = query.value(field.name);
//                 if (value.isValid()) {
//                     std::any anyVal;
//                     if (field.type == typeid(long long)) anyVal = value.toLongLong();
//                     else if (field.type == typeid(std::string)) anyVal = value.toString().toStdString();
//                     else if (field.type == typeid(QDateTime)) anyVal = value.toDateTime();
//                     else continue; // або лог помилки типу

//                     field.set(&msg, anyVal);
//                 }
//             }
//         } else {
//             return std::nullopt;
//         }

//         return msg;
//     }

//     void save(Message& message){
//         auto meta = MessageReflection::meta();
//         QSqlQuery query(db.getThreadDatabase());

//         QStringList columns;
//         QStringList placeholders;
//         QList<QVariant> values;

//         for (const auto& field : meta.fields) {
//             if (std::string(field.name) == "id") continue;

//             columns << field.name;
//             placeholders << "?";

//             std::any val = field.get(&message);
//             if (field.type == typeid(long long))
//                 values << QVariant::fromValue(std::any_cast<long long>(val));
//             else if (field.type == typeid(std::string))
//                 values << QString::fromStdString(std::any_cast<std::string>(val));
//             else if (field.type == typeid(QDateTime))
//                 values << std::any_cast<QDateTime>(val);
//         }

//         QString sql = QString("INSERT INTO message (%1) VALUES (%2)")
//                           .arg(columns.join(", "))
//                           .arg(placeholders.join(", "));
//         query.prepare(sql);

//         for (int i = 0; i < values.size(); ++i)
//             query.bindValue(i, values[i]);

//         if (!query.exec()) {
//             throw std::runtime_error(("Insert failed: " + query.lastError().text()).toStdString());
//         }

//         QVariant newId = query.lastInsertId();
//         if (newId.isValid()) {
//             std::any anyId = newId.toLongLong();
//             auto metaInfo = MessageReflection::meta().find("id");
//             if (metaInfo)
//                 metaInfo->set(&message, anyId);
//         } else {
//             throw std::runtime_error("Failed to retrieve inserted message ID");
//         }
//     }
// };



#endif // MESSAGE_H



