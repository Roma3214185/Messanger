#include <crow.h>

#include "Debug_profiling.h"
#include "NetworkFacade.h"
#include "NetworkManager.h"
#include "RabbitMQClient.h"
#include "config/ports.h"
#include "notificationservice/managers/NotificationOrchestrator.h"
#include "notificationservice/server.h"
#include "threadpool.h"
#include "SocketHandlersRepositoty.h"
#include "notificationservice/SocketRepository.h"
#include "notificationservice/IPublisher.h"
#include "notificationservice/ISubscriber.h"
#include "notificationservice/SocketNotifier.h"

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
  NetworkManager network_manager;
  SocketRepository socket_repository;

  RabbitNotificationPublisher publisher(&mq);
  SocketNotifier notifier(&socket_repository);

  NotificationOrchestrator notifManager(&network_manager, &publisher, &notifier);
  RabbitNotificationSubscriber subscriber(&mq, &notifManager);
  subscriber.subscribeAll();

  SocketHandlersRepository socket_handlers;
  socket_handlers.initHandlers(&publisher, &socket_repository);

  Server server(Config::Ports::notificationService, &socket_repository, &socket_handlers);
  server.run();
}
