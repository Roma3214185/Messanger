#include <QCoreApplication>

#include "Batcher.h"
#include "Debug_profiling.h"
#include "GeneratorId.h"
#include "GenericRepository.h"
#include "ProdConfigProvider.h"
#include "RabbitMQClient.h"
#include "SQLiteDataBase.h"
#include "SqlExecutor.h"
#include "interfaces/IThreadPool.h"
#include "messageservice/controller.h"
#include "messageservice/managers/MessageManager.h"
#include "messageservice/server.h"
#include "threadpool.h"

RabbitMQConfig getConfig(const ProdConfigProvider& provider) {
  RabbitMQConfig config;
  config.host     = "localhost";
  config.port     = provider.ports().rabitMQ;
  config.password = "guest";
  config.user     = "guest";
  return config;
}

std::unique_ptr<RabbitMQClient> createRabbitMQClient(const RabbitMQConfig config,
                                                     IThreadPool*         pool) {
  try {
    return std::make_unique<RabbitMQClient>(config, pool);
  } catch (const AmqpClient::AmqpLibraryException& e) {
    LOG_ERROR("Cannot connect to RabbitMQ: {}", e.what());
    return nullptr;
  }
}

int main(int argc, char* argv[]) {
  initLogger("MessageService");
  QCoreApplication a(argc, argv);
  SQLiteDatabase   bd("message_service_conn");
  if (!bd.initializeSchema()) {
    qFatal("Cannot initialise DB");
  }

  SqlExecutor        executor(bd);
  ThreadPool         pool;
  constexpr int      service_id = 3;
  GeneratorId        generator(service_id);
  GenericRepository  genetic_rep(bd, &executor, RedisCache::instance(), &pool);
  MessageManager     manager(&genetic_rep, &executor, &generator);
  ProdConfigProvider provider;
  RabbitMQConfig     config = getConfig(provider);
  auto               mq     = createRabbitMQClient(config, &pool);
  if (!mq) throw std::runtime_error("Cannot connect to RabbitMQ");

  Controller      controller(mq.get(), &manager, &pool);
  crow::SimpleApp app;
  Server          server(app, provider.ports().messageService, &controller);
  server.run();

  return a.exec();
}
