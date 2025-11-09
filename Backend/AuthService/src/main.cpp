#include <QCoreApplication>

#include "Debug_profiling.h"
#include "GenericRepository.h"
#include "RedisCache.h"
#include "SqlExecutor.h"
#include "authmanager.h"
#include "server.h"

const int AUTH_PORT = 8083;

void genereteKeys();

int main(int argc, char* argv[]) {
  QCoreApplication a(argc, argv);
  init_logger("AuthService");

  SQLiteDatabase    db;
  SqlExecutor       executor(db);
  GenericRepository rep(db, executor, RedisCache::instance());

  AuthManager manager(rep);

  Server server(AUTH_PORT, &manager);
  server.run();

  return 0;
}
