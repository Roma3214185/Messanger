#include <QCoreApplication>
#include <QDebug>
#include <event2/event.h>
#include <crow.h>
#include "Debug_profiling.h"
#include <crow/crow.h>
#include <QDateTime>
#include "rabbitmqclient.h"
#include "nlohmann/json.hpp"
#include "Headers/Message.h"
#include "notificationmanager.h"
#include "networkmanager.h"
#include "server.h"

const int NOTIFICATION_PORT = 8086;


int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);
    RabbitMQClient mq("localhost", "guest", "guest");
    SocketsManager sockManager;
    NetworkManager netManager;
    NotificationManager notifManager(mq, sockManager, netManager);

    Server server(NOTIFICATION_PORT, notifManager);
    server.run();
}
