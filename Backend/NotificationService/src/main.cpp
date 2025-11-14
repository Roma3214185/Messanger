#include <crow.h>

#include <QCoreApplication>

#include "Debug_profiling.h"
#include "RabbitMQClient.h"
#include "NetworkFacade.h"
#include "managers/notificationmanager.h"
#include "server.h"
#include "ports.h"
#include "NetworkManager.h"

int main(int argc, char* argv[]) {
  QCoreApplication    a(argc, argv);
  RabbitMQClient      mq("localhost", ports::RabitMqPort, "guest", "guest");
  SocketsManager      sockManager;
  NetworkManager network_manager;
  NetworkFacade net_repository = NetworkFactory::create(&network_manager);
  NotificationManager notifManager(&mq, sockManager, net_repository);
  Server              server(ports::NotificationServicePort, notifManager);
  server.run();
}
