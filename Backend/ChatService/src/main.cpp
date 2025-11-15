#include <crow.h>

#include <QCoreApplication>

#include "Debug_profiling.h"
#include "GenericRepository.h"
#include "SqlExecutor.h"
#include "chatservice/chatmanager.h"
#include "chatservice/controller.h"
#include "chatservice/server.h"
#include "NetworkManager.h"
#include "ProdConfigProvider.h"

int main(int argc, char* argv[]) {
  init_logger("ChatService");
  QCoreApplication  a(argc, argv);
  SQLiteDatabase    database;
  SqlExecutor       executor(database);
  GenericRepository genetic_rep(database, executor, RedisCache::instance());
  ChatManager       manager(&genetic_rep);
  NetworkManager    network_manager;
  ProdConfigProvider provider;
  Server            server(provider.ports().chatService, &manager, &network_manager);
  LOG_INFO("Chat service on port '{}'", provider.ports().chatService);
  server.run();
}
