#include <crow.h>

#include <QCoreApplication>

#include "Debug_profiling.h"
#include "GeneratorId.h"
#include "GenericRepository.h"
#include "NetworkFacade.h"
#include "NetworkManager.h"
#include "RedisCache.h"
#include "SQLiteDataBase.h"
#include "SqlExecutor.h"
#include "chatservice/chatcontroller.h"
#include "chatservice/chatmanager.h"
#include "chatservice/chatserver.h"
#include "config/ports.h"

int main(int argc, char *argv[]) {
  initLogger("ChatService");
  QCoreApplication a(argc, argv);

  SQLiteDatabase database("chat_service_conn");

  // if (!database.deleteTable(sqlite, "private_chats")) {
  //   LOG_ERROR("Not deleted private_chats table");
  // } {
  //   LOG_WARN("private_chats table is deleted");
  // }

  // if (!database.deleteTable(sqlite, "chats")) {
  //   LOG_ERROR("Not deleted chats table");
  // } {
  //   LOG_WARN("Chats table is deleted");
  // }

  // if (!database.deleteTable(sqlite, "chat_members")) {
  //   LOG_ERROR("Not deleted chat_members table");
  // } {
  //   LOG_WARN("chat_members table is deleted");
  // }

  if (!database.initializeSchema()) {
    qFatal("Cannot initialise DB");
  }

  SqlExecutor executor(database);
  constexpr int service_id = 2;
  GeneratorId generator(service_id);
  GenericRepository genetic_rep(&executor, RedisCache::instance());
  ChatManager manager(&genetic_rep, &generator);  // TODO: pass executor to mock
  NetworkManager network_manager;
  NetworkFacade facade = NetworkFactory::create(&network_manager);
  crow::SimpleApp app;
  ChatController controller(&manager, &facade);
  ChatServer server(app, Config::Ports::chatService, &controller);
  LOG_INFO("Chat service on port '{}'", Config::Ports::chatService);
  server.run();
}
