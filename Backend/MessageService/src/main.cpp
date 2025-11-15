#include <QCoreApplication>

#include "Batcher.h"
#include "Debug_profiling.h"
#include "GenericRepository.h"
#include "RabbitMQClient.h"
#include "SqlExecutor.h"
#include "messageservice/managers/MessageManager.h"
#include "messageservice/server.h"
#include "ProdConfigProvider.h"
#include "interfaces/IThreadPool.h"
#include "ThreadPool.h"
#include "messageservice/controller.h"

RabbitMQConfig getConfig(const ProdConfigProvider& provider) {
  RabbitMQConfig config;
  config.host = "localhost";
  config.port = provider.ports().rabitMQ;
  config.password = "guest";
  config.user = "guest";
  return config;
}

std::unique_ptr<RabbitMQClient> createRabbitMQClient(const RabbitMQConfig config, IThreadPool* pool) {
  try {
    return std::make_unique<RabbitMQClient>(config, pool);
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
  MessageManager  manager(&genetic_rep);
  ThreadPool pool;
  ProdConfigProvider provider;
  RabbitMQConfig config = getConfig(provider);
  auto mq = createRabbitMQClient(config, &pool);
  if(!mq) throw std::runtime_error("Cannot connect to RabbitMQ");

  Controller controller(mq.get(), &manager, &pool);
  Server server(provider.ports().messageService, &controller);
  server.run();

  return a.exec();
}
