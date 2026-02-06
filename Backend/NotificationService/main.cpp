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
#include "proxyclient.h"
#include "RealHttpClient.h"

RabbitMQConfig getConfig() {
  RabbitMQConfig config;
  config.host = "localhost";
  config.port = Config::Ports::rabitMQ;
  config.user = "guest";
  config.password = "guest";
  return config;
}

void initHandlers(SocketHandlers& handlers_, IPublisher *publisher, IUserSocketRepository *socket_repository) {
    handlers_["init"] = std::make_unique<InitMessageHandler>(socket_repository);
    handlers_["send_message"] = std::make_unique<SendMessageHandler>(publisher);
    handlers_["read_message"] = std::make_unique<MarkReadMessageHandler>(publisher);
    handlers_["save_reaction"] = std::make_unique<SaveMessageReactionHandler>(publisher);
    handlers_["delete_reaction"] = std::make_unique<DeleteMessageReactionHandler>(publisher);
}

int main() {
  initLogger("NotifiactionService");
  RabbitMQConfig config = getConfig();
  ThreadPool pool;
  RabbitMQClient mq(config, &pool);
  RealHttpClient client;
  ProxyClient proxy(&client);

  NetworkFacade network_manager(&proxy);
  SocketRepository socket_repository;

  RabbitNotificationPublisher publisher(&mq);
  SocketNotifier notifier(&socket_repository);

  NotificationOrchestrator notifManager(&network_manager, &publisher, &notifier);
  RabbitNotificationSubscriber subscriber(&mq, &notifManager);

  SocketHandlersRepository socket_handlers;
  SocketHandlers handlers;
  initHandlers(handlers, &publisher, &socket_repository);
  socket_handlers.setHandlers(std::move(handlers));

  Server server(Config::Ports::notificationService, &socket_repository, &socket_handlers, &subscriber);
  server.run();
}
