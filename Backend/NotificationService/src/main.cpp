#include <crow.h>

#include <QCoreApplication>

#include "Debug_profiling.h"
#include "RabbitMQClient.h"
#include "managers/networkmanager.h"
#include "managers/notificationmanager.h"
#include "server.h"
#include "ports.h"

int main(int argc, char* argv[]) {
  QCoreApplication    a(argc, argv);
  RabbitMQClient      mq("localhost", ports::RabitMqPort, "guest", "guest");
  SocketsManager      sockManager;
  NetworkManager      netManager;
  NotificationManager notifManager(&mq, sockManager, &netManager);
  Server              server(ports::NotificationServicePort, notifManager);
  server.run();
}
