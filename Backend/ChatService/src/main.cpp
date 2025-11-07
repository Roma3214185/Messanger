#include <crow.h>

#include <QCoreApplication>

#include "controller.h"
#include "database.h"
#include "Debug_profiling.h"
#include "server.h"
#include "chatmanager.h"
#include "Persistence/GenericRepository.h"
#include "SqlExecutor.h"

const int kChatServicePort = 8081;

int main(int argc, char* argv[]) {
  init_logger("ChatService");
  QCoreApplication a(argc, argv);
  SQLiteDatabase bd;
  SqlExecutor executor(bd);
  GenericRepository genetic_rep(executor, RedisCache::instance());
  ChatManager manager(&genetic_rep);
  Server server(kChatServicePort, &manager);
  LOG_INFO("Chat service on port '{}'", kChatServicePort);
  server.run();
}
