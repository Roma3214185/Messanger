#include <crow.h>
#include "src/DataBase/database.h"
#include "src/Controller/controller.h"
#include <QCoreApplication>
#include "Server/server.h"
#include "../../DebugProfiling/Debug_profiling.h"

const int CHAT_SERVICE_PORT = 8081;

int main(int argc, char *argv[]) {
    init_logger("ChatService");
    QCoreApplication a(argc, argv);
    DataBase db;
    //db.clearDataBase();
    Server server(CHAT_SERVICE_PORT, db);
    spdlog::info("Chat service on port '{}'", CHAT_SERVICE_PORT);
    server.run();
}


