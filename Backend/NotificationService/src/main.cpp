#include <crow.h>

#include "Debug_profiling.h"
#include "RabbitMQClient.h"
#include "NetworkFacade.h"
#include "notificationservice/managers/notificationmanager.h"
#include "notificationservice/server.h"
#include "notificationservice/managers/socketmanager.h"
#include "NetworkManager.h"
#include "ProdConfigProvider.h"
#include "ThreadPool.h"

RabbitMQConfig getConfig(const ProdConfigProvider& provider) {
  RabbitMQConfig config;
  config.host = "localhost";
  config.port = provider.ports().rabitMQ;
  config.user = "guest";
  config.password = "guest";
  return config;
}

int main() {
  ProdConfigProvider provider;
  RabbitMQConfig config = getConfig(provider);
  ThreadPool pool;
  RabbitMQClient      mq(config, &pool);
  SocketsManager      sockManager;
  NetworkManager network_manager;
  NetworkFacade net_repository = NetworkFactory::create(&network_manager);
  NotificationManager notifManager(&mq, &sockManager, net_repository);
  Server              server(provider.ports().notificationService, &notifManager);
  server.run();
}
