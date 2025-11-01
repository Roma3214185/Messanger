#include <crow.h>

#include <QCoreApplication>

#include "Controller/controller.h"
#include "DataBase/database.h"
#include "Debug_profiling.h"
#include "Server/server.h"
#include "ChatManager/chatmanager.h"
#include "GenericRepository/GenericRepository.h"

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
