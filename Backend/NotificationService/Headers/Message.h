#ifndef MESSAGE_H
#define MESSAGE_H

#include <QDateTime>
#include <string>

#include <crow/crow.h>
#include "nlohmann/json.hpp"

#include "Debug_profiling.h"

struct Message{
    long long id;
    long long chat_id;
    long long sender_id;
    std::string text;
    long long timestamp;
};

inline void to_json(nlohmann::json& j, const Message& m) {
    j = nlohmann::json{
        {"id", m.id},
        {"chat_id", m.chat_id},
        {"sender_id", m.sender_id},
        {"text", m.text},
        {"timestamp", m.timestamp}
    };
}

inline void from_json(const nlohmann::json& j, Message& u) {
    j.at("id").get_to(u.id);
    j.at("chat_id").get_to(u.chat_id);
    j.at("sender_id").get_to(u.sender_id);
    j.at("text").get_to(u.text);
    j.at("timestamp").get_to(u.timestamp);
}

inline crow::json::wvalue to_crow_json(const Message& m) {
    crow::json::wvalue j;
    LOG_INFO("[Message] id '{}' | chat_id '{}' | sender_id '{}' | text '{}' | timestamp '{}'", m.id, m.chat_id, m.sender_id, m.text, m.timestamp);
    j["id"] = m.id;
    j["chat_id"] = m.chat_id;
    j["sender_id"] = m.sender_id;
    j["text"] = m.text;
    j["timestamp"] = QDateTime::fromSecsSinceEpoch(m.timestamp).toString(Qt::ISODate).toStdString();
    return j;
}

inline Message from_crow_json(const crow::json::rvalue& j) {
    Message m;
    if(j.count("id")) m.id = j["id"].i();
    else m.id = 0;

    m.chat_id = j["chat_id"].i();
    m.sender_id = j["sender_id"].i();
    m.text = j["text"].s();

    if(j.count("timestamp")) {
        QString ts = QString::fromStdString(j["timestamp"].s());
        QDateTime dt = QDateTime::fromString(ts, Qt::ISODate);
        if(!dt.isValid()) {
            m.timestamp = QDateTime::currentDateTime().toSecsSinceEpoch();
        }else{
            m.timestamp = dt.toSecsSinceEpoch();
        }
    } else {
        m.timestamp = QDateTime::currentDateTime().toSecsSinceEpoch();
    }

    LOG_INFO("[Message from json] id '{}' | chat_id '{}' | sender_id '{}' | text '{}' | timestamp '{}'", m.id, m.chat_id, m.sender_id, m.text, m.timestamp);
    return m;
}

#endif // MESSAGE_H
