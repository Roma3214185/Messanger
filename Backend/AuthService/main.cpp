#include <QCoreApplication>

#include "authmanager.h"
#include "Debug_profiling.h"
#include "GenericRepository.h"
#include "server.h"

const int AUTH_PORT = 8083;

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);
    init_logger("AuthService");
    SQLiteDatabase db;
    GenericRepository rep(db);
    AuthManager manager(rep);

    Server server(AUTH_PORT, &manager);
    server.run();

    return 0;
}

