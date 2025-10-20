#include "notificationmanager.h"

//NotificationService::NotificationService() {}

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


void NotificationManager::notifyMessageRead(int chatId, const MessageStatus& status) {
    auto members = NetworkManager::getMembersOfChat(chatId);
    for (int userId : members) {
        if (userSockets.find(userId) == userSockets.end()) {
            crow::json::wvalue msgJson;
            msgJson["type"] = "message_read";
            msgJson["message_id"] = status.id;
            msgJson["reader_id"] = status.receiver_id;

            userSockets[userId]->send_text(msgJson.dump());
        }
    }
}

void NotificationManager::notifyNewMessages(Message msg, int toUser){
    PROFILE_SCOPE("NotificationManager::NotifyNewMessages");
    LOG_INFO("Chat id: '{}'; member is ", msg.chat_id, toUser);
    auto it = userSockets.find(toUser);

    if (it != userSockets.end()) {
        auto forwardMsg = to_crow_json(msg);
        forwardMsg["type"] = "message";
        it->second->send_text(forwardMsg.dump()); // don;t use dump??
        LOG_INFO("Forward message to id '{}'", toUser);
    } else {
        LOG_INFO("User offline, Message is saved to id '{}'", toUser);
    }
}
