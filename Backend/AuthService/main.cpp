#include <QCoreApplication>

#include "Debug_profiling.h"
#include "GenericRepository.h"
#include "RedisCache.h"
#include "SqlExecutor.h"
#include "authservice/authmanager.h"
#include "authservice/server.h"
#include "SqlExecutor.h"
#include "ProdConfigProvider.h"
#include "authservice/authcontroller.h"
#include "authservice/RealAuthoritizer.h"
#include "authservice/JwtGenerator.h"
#include "GeneratorId.h"

int main(int argc, char* argv[]) {
  QCoreApplication a(argc, argv);
  init_logger("AuthService");

  QSqlDatabase sqlite = QSqlDatabase::addDatabase("QSQLITE", "chat_service_conn");
  sqlite.setDatabaseName("chat_service.sqlite");

  if (!sqlite.open()) {
    qFatal("Cannot open DB");
  }

  SQLiteDatabase db(sqlite);
  if(!db.initializeSchema()) {
    qFatal("Cannot initialise DB");
  }

  SqlExecutor executor(db);
  constexpr int service_id = 1;
  GeneratorId id_generator(service_id);
  GenericRepository rep(db, &executor, RedisCache::instance(), &id_generator);

  AuthManager manager(rep);
  ProdConfigProvider provider;
  RealAuthoritizer authoritizer;
  JwtGenerator generator;
  AuthController controller(&manager, &authoritizer, &generator);
  crow::SimpleApp app;
  Server server(app, provider.ports().authService, &controller);
  server.run();

  return 0;
}
