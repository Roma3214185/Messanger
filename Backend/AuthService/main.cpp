#include "src/AuthManager/authmanager.h"
#include "src/DataBase/database.h"
#include "Server/server.h"
#include <QCoreApplication>

const int AUTH_PORT = 8083;

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);
    DataBase db;
    AuthManager manager(db);

    Server server(AUTH_PORT, &manager);
    server.run();

    return 0;
}

