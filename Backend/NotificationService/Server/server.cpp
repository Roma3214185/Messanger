#include "server.h"
#include <crow/crow.h>
#include "notificationmanager.h"

Server::Server(int port, NotificationManager& notifManager)
    : notifManager(notifManager)
    , notificationPort(port)
{

}

void Server::run(){
    LOG_INFO("Notication service is running on '{}'", notificationPort);
    app_.port(notificationPort).multithreaded().run();
}

void Server::initRoutes(){
    handleSocketRoutes();
}

void Server::handleSocketRoutes(){
    CROW_ROUTE(app_, "/ws").websocket(&app_)
        .onopen([&](crow::websocket::connection& conn) {
            LOG_INFO("Websocket is connected");
        })
        .onclose([&](crow::websocket::connection& conn, const std::string& reason, uint16_t code) {
            LOG_INFO("websocket disconnected, reason: '{}' and code '{}'", reason, code);
            notifManager.deleteConnections(&conn);
        })
        .onmessage([&](crow::websocket::connection& conn, const std::string& data, bool is_binary) {
            handleSocketOnMessage(conn, data, is_binary);
        });
}

void Server::handleSocketOnMessage(crow::websocket::connection &conn, const std::string &data, bool is_binary){
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
        notifManager.userConnected(userId, &conn);
        LOG_INFO("[onMessage] Socket is registered for userId '{}'", userId);
    }else if (msg["type"].s() == "send_message") {
        auto message = from_crow_json(msg);
        notifManager.onSendMessage(message);
    }else if(msg["type"].s() == "mark_read"){
        auto message = from_crow_json(msg);
        int readBy = msg["receiver_id"].i();
        notifManager.onMarkReadMessage(message, readBy);
    }else{
        LOG_ERROR("[onMessage] Invalid type");
    }
}
