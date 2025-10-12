#include <crow.h>
#include "src/DataBase/database.h"
#include "src/Controller/controller.h"
#include <QCoreApplication>
#include "Server/server.h"

const int CHAT_SERVICE_PORT = 8081;

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);
    DataBase db;
    Server server(CHAT_SERVICE_PORT, db);
    server.run();
}


