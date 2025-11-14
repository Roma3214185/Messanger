#include <QCoreApplication>

#include "Batcher.h"
#include "Debug_profiling.h"
#include "GenericRepository.h"
#include "RabbitMQClient.h"
#include "SqlExecutor.h"
#include "managers/MessageManager.h"
#include "managers/notificationmanager.h"
#include "server.h"
#include "ProdConfigProvider.h"

RabbitMQConfig getConfig(const ProdConfigProvider& provider) {
  RabbitMQConfig config;
  config.host = "localhost";
  config.port = provider.ports().rabitMQ;
  config.password = "guest";
  config.user = "guest";
  return config;
}

std::unique_ptr<RabbitMQClient> createRabbitMQClient(const RabbitMQConfig config) {
  try {
    return std::make_unique<RabbitMQClient>(config);
  } catch (const AmqpClient::AmqpLibraryException& e) {
    LOG_ERROR("Cannot connect to RabbitMQ: {}", e.what());
    return nullptr;
  }
}

int main(int argc, char* argv[]) {
  init_logger("MessageService");
  QCoreApplication  a(argc, argv);
  SQLiteDatabase    bd;
  SqlExecutor       executor(bd);
  GenericRepository genetic_rep(bd, executor, RedisCache::instance());

  SaverBatcher<Message>         message_saver_batcher(genetic_rep);
  SaverBatcher<MessageStatus>   message_status_saver_batcher(genetic_rep);
  DeleterBatcher<Message>       message_delete_batcher(genetic_rep);
  DeleterBatcher<MessageStatus> message_status_delete_batcher(genetic_rep);

  Batcher<Message>       message_batcher(message_saver_batcher, message_delete_batcher);
  Batcher<MessageStatus> message_status_batcher(message_status_saver_batcher,
                                                message_status_delete_batcher);

  MessageManager  manager(&genetic_rep, &message_batcher, &message_status_batcher);

  ProdConfigProvider provider;
  RabbitMQConfig config = getConfig(provider);
  auto mq = createRabbitMQClient(config);
  if(!mq) throw std::runtime_error("Cannot connect to RabbitMQ");

  Server server(provider.ports().messageService, &manager, mq.get());
  server.run();

  return a.exec();
}
