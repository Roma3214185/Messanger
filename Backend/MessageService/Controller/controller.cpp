#include "controller.h"
#include <QDebug>
#include "NetworkManager.h"
#include "nlohmann/json.hpp"

inline crow::json::wvalue to_crow_json(const Message& m) {
    crow::json::wvalue j;
    LOG_INFO("[Message] id '{}' | chat_id '{}' | sender_id '{}' | text '{}' | timestamp '{}'", m.id, m.chat_id, m.sender_id, m.text, m.timestamp);
    j["id"] = m.id;
    j["chat_id"] = m.chat_id;
    j["sender_id"] = m.sender_id;
    j["text"] = m.text;
    j["timestamp"] = QDateTime::fromSecsSinceEpoch(m.timestamp).toString(Qt::ISODate).toStdString();
    LOG_INFO("Timestamp = ", QDateTime::fromSecsSinceEpoch(m.timestamp).toString(Qt::ISODate).toStdString());
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



Controller::Controller(crow::SimpleApp& app, MessageManager& manager)
    : app_(app)
    , manager(manager)
{
    // auto initial = db.initialDb();
    // if(!initial) {
    //     qDebug() << "[ERROR] bd isn't initial";
    //     throw std::runtime_error("BD not initial");
    // }
}

void Controller::handleRoutes(){
    handleGetMessagesFromChat();
    handleSocket();
}

void Controller::handleSocket(){
CROW_ROUTE(app_, "/ws")
    .websocket(&app_)
    .onopen([&](crow::websocket::connection& conn) {
        LOG_INFO("Websocket is connected");
    })
    .onclose([&](crow::websocket::connection& conn, const std::string& reason, uint16_t code) {
        LOG_INFO("websocket disconnected, reason: '{}' and code '{}'", reason, code);
        std::lock_guard<std::mutex> lock(socketMutex);
        for (auto it = userSockets.begin(); it != userSockets.end(); ++it) {
            if (it->second == &conn) {
                userSockets.erase(it);
                break;
            }
        }
    })
    .onmessage([&](crow::websocket::connection& conn, const std::string& data, bool is_binary) {
        auto msg = crow::json::load(data);

        if (!msg) {
            LOG_ERROR("[onMessage] Failed in loading message");
            return;
        }

        if (msg.has("type") && msg["type"].s() == "init") {
            int userId = msg["userId"].i();
            userConnected(userId, &conn);
            LOG_INFO("[onMessage] Socket is registered for userId '{}'", userId);
        } else if (msg.has("type") && msg["type"].s() == "send_message") {
            auto message = from_crow_json(msg);
            onSendMessage(message);
        }else{
            LOG_ERROR("[onMessage] Invalid type");
        }
    });
}

void Controller::handleGetMessagesFromChat(){
CROW_ROUTE(app_, "/messages/<int>").methods(crow::HTTPMethod::GET)
    ([&](const crow::request& req, int chatId) {
        PROFILE_SCOPE("/messages/id");

        int limit = req.url_params.get("limit") ? std::stoi(req.url_params.get("limit")) : INT_MAX;
        int beforeId = req.url_params.get("beforeId") ? std::stoi(req.url_params.get("beforeId")) : 0;

        LOG_INFO("For id '{}' limit is '{}' and beforeId is '{}'", chatId, limit, beforeId);

        //make check if u have acess to ges these messagess

        auto messages = manager.getChatMessages(chatId, limit, beforeId);

        LOG_INFO("For chat '{}' finded '{}' messages", chatId, messages.size());
        crow::json::wvalue res = crow::json::wvalue::list();
        int i = 0;
        for (const auto& msg : messages) {
            auto m = to_crow_json(msg);
            res[i++] = std::move(m);
        }

        return crow::response(200, res);
    });
}

void Controller::userConnected(int userId, crow::websocket::connection* conn){
    PROFILE_SCOPE("Controller::userConnected");
    userSockets[userId] = conn;

    auto pendingMessages = manager.getUndeliveredMessages(userId);
    LOG_INFO(" UserId '{}' has unreaden '{}' messages", userId, pendingMessages.size());
    for (auto& msgStatus : pendingMessages) {
        auto msg = manager.getMessage(msgStatus.id);
        if(msg) userSockets[userId]->send_text(msg->text);
        else LOG_ERROR("Message not found for '{}'", msgStatus.id);
        //manager.markDelivered(msg); // and not readen??
    }
}

void Controller::onSendMessage(Message msg){
    PROFILE_SCOPE("Controller::onSendMessage");
    LOG_INFO("Send message from '{}' to chatId '{}' (text: '{}')", msg.sender_id, msg.chat_id, msg.text);

    //try catch
    manager.saveMessage(msg);
    LOG_INFO("Message('{}') is saved with id '{}'", msg.text, msg.id);

    auto members_of_chat = NetworkManager::getMembersOfChat(msg.chat_id);
    LOG_INFO("For chat id '{}' finded '{}' members", msg.chat_id, members_of_chat.size());

    for(auto toUser: members_of_chat){
        LOG_INFO("Chat id: '{}'; member is ", msg.chat_id, toUser);
        auto it = userSockets.find(toUser);

        MessageStatus msgStatus{
            .id = msg.id,
            .receiver_id = toUser,
            .is_read = false
        };



        if (it != userSockets.end()) {
            //manager.markDelivered(msg);
            auto forwardMsg = to_crow_json(msg);
            forwardMsg["type"] = "message";
            it->second->send_text(forwardMsg.dump()); // don;t use dump??
            LOG_INFO("Forward message to id '{}'", toUser);
            msgStatus.is_read = true;
            manager.saveMessageStatus(msgStatus);
        } else {
            LOG_INFO("User offline, Message is saved to id '{}'", toUser);
            manager.saveMessageStatus(msgStatus);
        }
    }
}
