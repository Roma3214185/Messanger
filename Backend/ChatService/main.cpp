#include <crow.h>
#include <QCoreApplication>

#include "Controller/controller.h"
#include "DataBase/database.h"
#include "Debug_profiling.h"
#include "Server/server.h"

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


