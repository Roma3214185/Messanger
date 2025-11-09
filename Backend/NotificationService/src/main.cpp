#include <crow.h>

#include <QCoreApplication>

#include "Debug_profiling.h"
#include "RabbitMQClient.h"
#include "managers/networkmanager.h"
#include "managers/notificationmanager.h"
#include "server.h"

const int     NOTIFICATION_PORT = 8086;
constexpr int kRabitMQPort      = 5672;

int main(int argc, char* argv[]) {
  QCoreApplication    a(argc, argv);
  RabbitMQClient      mq("localhost", kRabitMQPort, "guest", "guest");
  SocketsManager      sockManager;
  NetworkManager      netManager;
  NotificationManager notifManager(mq, sockManager, netManager);
  Server              server(NOTIFICATION_PORT, notifManager);
  server.run();
}
