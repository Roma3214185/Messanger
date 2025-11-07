#include <crow.h>

#include <QCoreApplication>

#include "controller.h"
#include "database.h"
#include "Debug_profiling.h"
#include "server.h"
#include "chatmanager.h"
#include "Persistence/GenericRepository.h"

const int kChatServicePort = 8081;

int main(int argc, char* argv[]) {
  init_logger("ChatService");
  QCoreApplication a(argc, argv);
  //DataBase db;
  SQLiteDatabase database;
  GenericRepository repository(database);
  ChatManager manager(&repository);
  Server server(kChatServicePort, &manager);
  LOG_INFO("Chat service on port '{}'", kChatServicePort);
  server.run();
}
