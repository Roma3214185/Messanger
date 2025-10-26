#ifndef MESSAGESTATUS_H
#define MESSAGESTATUS_H

#include <QDateTime>

#include "nlohmann/json.hpp"

struct MessageStatus {
    long long id;
    long long receiver_id;
    bool is_read = false;
    long long read_at;
};

inline void to_json(nlohmann::json& j, const MessageStatus& m) {
    j = nlohmann::json{
        {"id", m.id},
        {"receiver_id", m.receiver_id},
        {"is_read", m.is_read},
        {"read_at", m.read_at}
    };
}

inline void from_json(const nlohmann::json& j, MessageStatus& u) {
    j.at("id").get_to(u.id);
    j.at("receiver_id").get_to(u.receiver_id);
    j.at("is_read").get_to(u.is_read);
    j.at("read_at").get_to(u.read_at);
}



#endif // MESSAGESTATUS_H
