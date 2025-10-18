#include "controller.h"
#include <QDebug>
#include "NetworkManager.h"
#include "nlohmann/json.hpp"

inline crow::json::wvalue to_crow_json(const Message& m) {
    crow::json::wvalue j;
    qDebug() << "[INFO] Message to crow id:" << m.id;
    qDebug() << "[INFO] Message to crow chat_id:" << m.chat_id;
    qDebug() << "[INFO] Message to crow sender_id:" << m.sender_id;
    qDebug() << "[INFO] Message to crow text:" << m.text;
    qDebug() << "[INFO] Message to crow timestamp:" << m.timestamp.toString(Qt::ISODate).toStdString();
    j["id"] = m.id;
    j["chat_id"] = m.chat_id;
    j["sender_id"] = m.sender_id;
    j["text"] = m.text;
    j["timestamp"] = m.timestamp.toString(Qt::ISODate).toStdString(); // ✅ ISO 8601 string
    return j;
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
            int fromUser = msg["sender_id"].i();
            int chatId = msg["chat_id"].i();
            std::string text = msg["text"].s();
            onSendMessage(fromUser, chatId, text);
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

void Controller::onSendMessage(int fromUser, int chatId, std::string text){
    PROFILE_SCOPE("Controller::onSendMessage");
    LOG_INFO("Send message from '{}' to chatId '{}' (text: '{}')", fromUser, chatId, text);
    Message msg{
        .chat_id = chatId,
        .sender_id = fromUser,
        .text = text,
        .timestamp = QDateTime::currentDateTime()
    };

    //try catch
    manager.saveMessage(msg); // messageStatus??
    LOG_INFO("Message('{}') is saved with id '{}'", msg.text, msg.id);

    auto members_of_chat = NetworkManager::getMembersOfChat(chatId);
    LOG_INFO("For chat id '{}' finded '{}' members", chatId, members_of_chat.size());

    for(auto toUser: members_of_chat){
        LOG_INFO("Chat id: '{}'; member is ", chatId, toUser);
        auto it = userSockets.find(toUser);

        MessageStatus msgStatus{
            .id = msg.id,
            .receiver_id = toUser,
            .is_read = false
        };

        manager.saveMessageStatus(msgStatus);

        if (it != userSockets.end()) {
            //manager.markDelivered(msg);
            auto forwardMsg = to_crow_json(msg);
            forwardMsg["type"] = "message";
            it->second->send_text(forwardMsg.dump()); // don;t use dump??
            LOG_INFO("Forward message to id '{}'", toUser);
        } else {
            LOG_INFO("User offline, Message is saved to id '{}'", toUser);
        }
    }
}
