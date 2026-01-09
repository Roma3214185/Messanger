#include <crow.h>

#include "Debug_profiling.h"
#include "NetworkFacade.h"
#include "NetworkManager.h"
#include "RabbitMQClient.h"
#include "notificationservice/managers/notificationmanager.h"
#include "notificationservice/managers/socketmanager.h"
#include "notificationservice/server.h"
#include "threadpool.h"

RabbitMQConfig getConfig() {
  RabbitMQConfig config;
  config.host = "localhost";
  config.port = Config::Ports::rabitMQ;
  config.user = "guest";
  config.password = "guest";
  return config;
}

int main() {
  initLogger("NotifiactionService");
  RabbitMQConfig config = getConfig();
  ThreadPool pool;
  RabbitMQClient mq(config, &pool);
  SocketsManager sockManager;
  NetworkManager network_manager;
  NetworkFacade net_repository = NetworkFactory::create(&network_manager);
  NotificationManager notifManager(&mq, &sockManager, net_repository);
  Server server(Config::Ports::notificationService, &notifManager);
  server.run();
}
