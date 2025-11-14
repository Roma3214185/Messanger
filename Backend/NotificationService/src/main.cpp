#include <crow.h>
#include <QCoreApplication>

#include "Debug_profiling.h"
#include "RabbitMQClient.h"
#include "NetworkFacade.h"
#include "managers/notificationmanager.h"
#include "server.h"
#include "NetworkManager.h"
#include "ProdConfigProvider.h"

RabbitMQConfig getConfig(const ProdConfigProvider& provider) {
  RabbitMQConfig config;
  config.host = "localhost";
  config.port = provider.ports().rabitMQ;
  config.user = "guest";
  config.password = "guest";
  return config;
}

int main(int argc, char* argv[]) {
  QCoreApplication    a(argc, argv);
  ProdConfigProvider provider;
  RabbitMQConfig config = getConfig(provider);
  RabbitMQClient      mq(config);
  SocketsManager      sockManager;
  NetworkManager network_manager;
  NetworkFacade net_repository = NetworkFactory::create(&network_manager);
  NotificationManager notifManager(&mq, sockManager, net_repository);
  Server              server(provider.ports().notificationService, notifManager);
  server.run();
}
