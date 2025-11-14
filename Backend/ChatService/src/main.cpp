#include <crow.h>

#include <QCoreApplication>

#include "Debug_profiling.h"
#include "GenericRepository.h"
#include "SqlExecutor.h"
#include "chatmanager.h"
#include "controller.h"
#include "database.h"
#include "server.h"
#include "NetworkManager.h"
#include "ports.h"

const int kChatServicePort = 8081;

int main(int argc, char* argv[]) {
  init_logger("ChatService");
  QCoreApplication  a(argc, argv);
  SQLiteDatabase    database;
  SqlExecutor       executor(database);
  GenericRepository genetic_rep(database, executor, RedisCache::instance());
  ChatManager       manager(&genetic_rep);
  NetworkManager    network_manager;
  Server            server(ports::ChatServicePort, &manager, &network_manager);
  LOG_INFO("Chat service on port '{}'", ports::ChatServicePort);
  server.run();
}
