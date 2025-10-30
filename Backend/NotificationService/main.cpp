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

    //crow::SimpleApp app_;
    RabbitMQClient mq("localhost", kRabitMQPort, "guest", "guest");

    //RabbitMQClient mq("localhost", "guest", "guest");

    SocketsManager sockManager;
    NetworkManager netManager;
    NotificationManager notifManager(mq, sockManager, netManager);
    Server server(NOTIFICATION_PORT, notifManager);
    server.run();
}
