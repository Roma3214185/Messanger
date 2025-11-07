#include <QCoreApplication>

#include "authmanager.h"
#include "Debug_profiling.h"
#include "Persistence/GenericRepository.h"
#include "server.h"
#include "SqlExecutor.h"
#include "RedisCache.h"

const int AUTH_PORT = 8083;

void genereteKeys();

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);
    init_logger("AuthService");

    SQLiteDatabase db;
    SqlExecutor executor(db);
    GenericRepository rep(executor, RedisCache::instance());

    AuthManager manager(rep);

    Server server(AUTH_PORT, &manager);
    server.run();

    return 0;
}

