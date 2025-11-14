#include <crow.h>
#include <QCoreApplication>

#include "Debug_profiling.h"
#include "RabbitMQClient.h"
#include "NetworkFacade.h"
#include "managers/notificationmanager.h"
#include "server.h"
#include "NetworkManager.h"
#include "ProdConfigProvider.h"

int main(int argc, char* argv[]) {
  QCoreApplication    a(argc, argv);
  ProdConfigProvider provider;
  RabbitMQClient      mq("localhost", provider.ports().rabitMQ, "guest", "guest");
  SocketsManager      sockManager;
  NetworkManager network_manager;
  NetworkFacade net_repository = NetworkFactory::create(&network_manager);
  NotificationManager notifManager(&mq, sockManager, net_repository);
  Server              server(provider.ports().notificationService, notifManager);
  server.run();
}
