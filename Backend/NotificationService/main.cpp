#include <QCoreApplication>
#include <crow.h>

#include "Debug_profiling.h"
#include "notificationmanager.h"
#include "networkmanager.h"
#include "rabbitmqclient.h"
#include "server.h"

const int NOTIFICATION_PORT = 8086;
constexpr int kRabitMQPort = 5672;

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);
    crow::SimpleApp app_;
    RabbitMQClient mq("localhost", kRabitMQPort, "guest", "guest");
    SocketsManager sockManager;
    NetworkManager netManager;
    NotificationManager notifManager(mq, sockManager, netManager);



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
    });

    LOG_INFO("Notication service is running on '{}'", NOTIFICATION_PORT);
    app_.port(NOTIFICATION_PORT).multithreaded().run();
}
