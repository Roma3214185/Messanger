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
  initLogger("AuthService");

  // if(db.deleteTable(sqlite, "users")) {
  //   LOG_INFO("[DELETE] users table");
  // } else {
  //   LOG_ERROR("[DELETE] users table failed");
  // }

  SQLiteDatabase db("auth_service_conn");
  if(!db.initializeSchema()) {
    qFatal("Cannot initialise DB");
  }

  SqlExecutor executor(db);
  constexpr int service_id = 1;
  GeneratorId id_generator(service_id);
  GenericRepository rep(db, &executor, RedisCache::instance());

  AuthManager manager(rep, &id_generator);
  ProdConfigProvider provider;
  RealAuthoritizer authoritizer;
  JwtGenerator generator;
  AuthController controller(&manager, &authoritizer, &generator);
  crow::SimpleApp app;
  Server server(app, provider.ports().authService, &controller, &generator);

  // if(!server.generateKeys()) {
  //   qFatal("Cannot generate keys");
  // }
  server.initRoutes();
  server.run();

  return 0;
}
