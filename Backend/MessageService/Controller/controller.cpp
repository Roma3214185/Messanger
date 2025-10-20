#include "controller.h"
#include <QDebug>
#include "NetworkManager.h"
#include "nlohmann/json.hpp"
#include "jwt-cpp/jwt.h"

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

inline constexpr const char* SECRET_KEY = "super_secret_key";
inline constexpr const char* ISSUER = "my-amazon-clone";

inline std::optional<int> verifyTokenAndGetUserId(const std::string& token) {
    try {
        auto decoded = jwt::decode(token);
        auto verifier = jwt::verify()
                            .allow_algorithm(jwt::algorithm::hs256{SECRET_KEY})
                            .with_issuer(ISSUER);
        verifier.verify(decoded);

        int userId = std::stoll(decoded.get_payload_claim("sub").as_string());
        LOG_INFO("Token is verified, id is '{}'", userId);
        return userId;
    } catch (const std::exception& e) {
        LOG_ERROR("Error verifying token, error: {}", e.what());
        return std::nullopt;
    } catch (...) {
        LOG_ERROR("Unknown error verifying token");
        return std::nullopt;
    }
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



Controller::Controller(crow::SimpleApp& app, MessageManager& manager, NotificationManager& notifManager)
    : app_(app)
    , manager(manager)
    , notifManager(notifManager)
{

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
        notifManager.deleteConnections(&conn);
    })
    .onmessage([&](crow::websocket::connection& conn, const std::string& data, bool is_binary) {
        auto msg = crow::json::load(data);

        if (!msg) {
            LOG_ERROR("[onMessage] Failed in loading message");
            return;
        }

        if (!msg.has("type")){
            LOG_ERROR("[onMessage] No type");
            return;
        }


        if(msg["type"].s() == "init") {
            int userId = msg["userId"].i();
            userConnected(userId, &conn);
            LOG_INFO("[onMessage] Socket is registered for userId '{}'", userId);
        } else if (msg["type"].s() == "send_message") {
            auto message = from_crow_json(msg);
            onSendMessage(message);
        }else if(msg["type"].s() == "mark_read"){
            auto message = from_crow_json(msg);
            int readBy = msg["receiver_id"].i();
            onMarkReadMessage(message, readBy);
        }else{
            LOG_ERROR("[onMessage] Invalid type");
        }
    });
}

void Controller::handleGetMessagesFromChat(){
CROW_ROUTE(app_, "/messages/<int>").methods(crow::HTTPMethod::GET)
    ([&](const crow::request& req, int chatId) {
        PROFILE_SCOPE("/messages/id");
        std::string token = getToken(req);
        std::optional<int> myId = verifyTokenAndGetUserId(token);
        if(!myId){         //make check if u have acess to ges these messagess
            LOG_ERROR("Can't verify token");
            return crow::response(500);
        }

        int limit = req.url_params.get("limit") ? std::stoi(req.url_params.get("limit")) : INT_MAX;
        int beforeId = req.url_params.get("beforeId") ? std::stoi(req.url_params.get("beforeId")) : 0;

        LOG_INFO("For id '{}' limit is '{}' and beforeId is '{}'", chatId, limit, beforeId);


        auto messages = manager.getChatMessages(chatId, limit, beforeId);

        auto allMessagesStatus = manager.getMessageStatusDebug();
        for(auto status: allMessagesStatus){
            LOG_INFO("[STATUS] id = '{}' and receiver_id = '{}' and is-read = '{}'", status.id, status.receiver_id, status.is_read);
        }

        LOG_INFO("For chat '{}' finded '{}' messages", chatId, messages.size());
        crow::json::wvalue res = crow::json::wvalue::list();
        int i = 0;
        for (const auto& msg : messages) {
            auto jsonObject = to_crow_json(msg);
            std::optional<MessageStatus> status_mine_message = manager.getMessageStatus(msg.id, *myId);
            LOG_INFO("Get message status for '{}' and receiver_id '{}'", msg.id, *myId);
            if(!status_mine_message) {
                LOG_ERROR("status_mine_message = false");
                continue;
            }
            jsonObject["readed_by_me"] = status_mine_message->is_read;

            //listOfMessages = manager.getMessageStatus(msg.id)
            //jsonObject["readed_by_me"] = listOfMessages.receivers.count(myId)
            //jsonObject["readed_count"] = listOfMessages.count.is_read

            res[i++] = std::move(jsonObject);
        }

        return crow::response(200, res);
    });
}

std::string Controller::getToken(const crow::request& req){
    return req.get_header_value("Authorization");
}

void Controller::userConnected(int userId, crow::websocket::connection* conn){
    notifManager.saveConnections(userId, conn);
    // notify users who communicate with this user
}

void Controller::onMarkReadMessage(Message message, int readBy){
    MessageStatus status{
        .id = message.id,
        .receiver_id = readBy,
        .is_read = true,
        .read_at = QDateTime::currentDateTime().toSecsSinceEpoch()
    };
    manager.saveMessageStatus(status);
    notifManager.notifyMessageRead(message.id, status);
}

void Controller::onSendMessage(Message msg){
    PROFILE_SCOPE("Controller::onSendMessage");
    LOG_INFO("Send message from '{}' to chatId '{}' (text: '{}')", msg.sender_id, msg.chat_id, msg.text);

    //try catch
    manager.saveMessage(msg);
    LOG_INFO("Message('{}') is saved with id '{}'", msg.text, msg.id);


    auto members_of_chat = NetworkManager::getMembersOfChat(msg.chat_id);
    qDebug() << "Chat members is " << members_of_chat.size();
    LOG_INFO("For chat id '{}' finded '{}' members", msg.chat_id, members_of_chat.size());


    for(auto toUser: members_of_chat){
        LOG_INFO("Chat id: '{}'; member is ", msg.chat_id, toUser);

        MessageStatus msgStatus{
            .id = msg.id,
            .receiver_id = toUser,
            .is_read = false
        };
        manager.saveMessageStatus(msgStatus);
        notifManager.notifyNewMessages(msg, toUser);
    }
}
