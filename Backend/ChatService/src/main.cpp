#include <crow.h>

#include <QCoreApplication>

#include "Debug_profiling.h"
#include "GenericRepository.h"
#include "SqlExecutor.h"
#include "chatservice/chatmanager.h"
#include "chatservice/chatcontroller.h"
#include "chatservice/chatserver.h"
#include "NetworkManager.h"
#include "ProdConfigProvider.h"
#include "NetworkFacade.h"

int main(int argc, char* argv[]) {
  init_logger("ChatService");
  QCoreApplication  a(argc, argv);
  SQLiteDatabase    database;
  SqlExecutor       executor(database);
  GenericRepository genetic_rep(database, &executor, RedisCache::instance());
  ChatManager       manager(&genetic_rep); //TODO: pass executor to mock
  NetworkManager    network_manager;
  ProdConfigProvider provider;
  NetworkFacade facade = NetworkFactory::create(&network_manager);
  crow::SimpleApp app;
  ChatController controller(&manager, &facade);
  ChatServer            server(app, provider.ports().chatService, &controller);
  LOG_INFO("Chat service on port '{}'", provider.ports().chatService);
  server.run();
}
