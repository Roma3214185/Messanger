#include "controller.h"
#include <QDebug>
#include "NetworkManager.h"

Controller::Controller(crow::SimpleApp& app, DataBase& dataBase)
    : app_(app)
    , db(dataBase)
{
    auto initial = db.initialDb();
    if(!initial) {
        qDebug() << "[ERROR] bd isn't initial";
        throw std::runtime_error("BD not initial");
    }
}

void Controller::handleRoutes(){
    handleGetMessagesFromChat();
    handleSocket();
}

void Controller::handleSocket(){
CROW_ROUTE(app_, "/ws")
    .websocket(&app_)
    .onopen([&](crow::websocket::connection& conn) {
        qDebug() << "[INFO] WebSocket connected\n";
    })
    .onclose([&](crow::websocket::connection& conn, const std::string& reason, uint16_t code) {
        qDebug() << "[INFO] WebSocket disconnected: " << reason << " (code: " << code << ")\n";

        std::lock_guard<std::mutex> lock(socketMutex);
        for (auto it = userSockets.begin(); it != userSockets.end(); ++it) {
            if (it->second == &conn) {
                userSockets.erase(it);
                break;
            }
        }
    })
    .onmessage([&](crow::websocket::connection& conn, const std::string& data, bool is_binary) {
        qDebug() << "[INFO] Received message: " << data << "\n";
        auto msg = crow::json::load(data);

        if (!msg) {
            qDebug() << "[ERROR] Failed in loading in MSG";
            return;
        }

        if (msg.has("type") && msg["type"].s() == "init") {
            int userId = msg["userId"].i();
            userConnected(userId, &conn);
            qDebug() << "[INFO] Socket is registered for userId: " << userId;
        } else if (msg.has("type") && msg["type"].s() == "send_message") {
            int fromUser = msg["sender_id"].i();
            int chatId = msg["chat_id"].i();
            std::string text = msg["text"].s();
            onSendMessage(fromUser, chatId, text);
        }
    });
}

void Controller::handleGetMessagesFromChat(){
CROW_ROUTE(app_, "/messages/<int>").methods(crow::HTTPMethod::GET)
    ([&](const crow::request& req, int chatId) {
        //make check if u have acess to ges these messagess
        qDebug() << "[INFO] get all messages from chat " << chatId;

        // (?) another route to return messages only already readed by user
        //(?) new messages he will receive from socket ???

        auto messages = db.getChatMessages(chatId);
        crow::json::wvalue res = crow::json::wvalue::list();
        int i = 0;
        for (const auto& msg : messages) {
            qDebug() << "TIME: " << msg.timestamp;
            crow::json::wvalue m;
            m["message_id"] = msg.id;
            m["chat_id"]    = msg.chatId;
            m["sender_id"]  = msg.senderId;
            m["text"]       = msg.text;

            QDateTime dt = QDateTime::fromString(msg.timestamp, Qt::ISODate);
            if (!dt.isValid()) {
                qWarning() << "[WARN] Invalid timestamp:" << msg.timestamp;
            }

            m["timestamp"] = dt.toString(Qt::ISODate).toStdString();

            qDebug() << "return id: " << msg.id << "; sender_id: " << msg.senderId << "; text = " << msg.text << "timestamp = " << dt.toString(Qt::ISODate).toStdString();
            res[i++] = std::move(m);
        }

        return crow::response(200, res);
    });
}

void Controller::userConnected(int userId, crow::websocket::connection* conn){
    userSockets[userId] = conn;

    auto pendingMessages = db.getUndeliveredMessages(userId); //а якшо я загружаю всі повідомлення при вході, тоді undeliveredMessages повторюються???
    qDebug() << "[INFO] UserId: " << userId << " has unreaden " << pendingMessages.size();
    for (const auto& msg : pendingMessages) {
        userSockets[userId]->send_text(msg.text);
        db.markDelivered(msg.id);
    }
}

void Controller::onSendMessage(int fromUser, int chatId, std::string text){
    qDebug() << "[INFO] Send message from" << fromUser << "to chatId" << chatId << ": " << text;

    std::optional<int> id = db.addMsgToDatabase(text, fromUser, chatId);
    if (!id) {
        qDebug() << "[ERROR] Failed to save message to DB:" << text;
        return;
    }

    qDebug() << "Message " << text << " has id: " << *id;

    QVector<int> members_of_chat = NetworkManager::getMembersOfChat(chatId);


    for(auto toUser: members_of_chat){
        qDebug() << "ID to send: " << toUser;
        auto it = userSockets.find(toUser);

        if (it != userSockets.end()) {
            db.saveMessage(*id, fromUser, toUser, text, true);
            crow::json::wvalue forwardMsg;
            forwardMsg["type"] = "message";
            forwardMsg["message_id"] = *id;
            forwardMsg["chat_id"] = chatId;
            forwardMsg["sender_id"] = fromUser;
            forwardMsg["text"] = text;
            forwardMsg["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate).toStdString();

            it->second->send_text(forwardMsg.dump());
            qDebug() << "[INFO] Forwarded message to user" << toUser;
        } else {
            qDebug() << "[INFO] User" << toUser << "not connected. I will save your message.";

            db.saveMessage(*id, fromUser, toUser, text, false);
        }
    }
}
