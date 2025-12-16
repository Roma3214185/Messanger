#include <benchmark/benchmark.h>
#include <QSqlQuery>
#include <QCoreApplication>

#include "SQLiteDataBase.h"
#include "authservice/authmanager.h"
#include "mocks.h"

struct TestAuthManager : public AuthManager {
    using AuthManager::AuthManager;
    using AuthManager::findUserByEmail;
};

QSqlDatabase prepareDatabase()
{
  QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
  db.setDatabaseName(":memory:");   // in-memory DB (fast, isolated)

  if (!db.open()) {
    qFatal("Cannot open database: %s",
           qPrintable(db.lastError().text()));
  }

  return db;
}

void createSchema(QSqlDatabase& db) {
  QSqlQuery q(db);

  q.exec(R"(
        CREATE TABLE users (
            id INTEGER PRIMARY KEY,
            email TEXT NOT NULL,
            name TEXT NOT NULL,
            tag TEXT NOT NULL
        )
    )");

  //q.exec("CREATE INDEX idx_users_age ON users(age)");
}

void seedData(QSqlDatabase& db, int rows = 1000)
{
  QSqlQuery q(db);

  q.prepare(R"(
        INSERT INTO users (id, email, name, tag)
        VALUES (?, ?, ?, ?)
    )");

  for (int i = 0; i < rows; ++i) {
    q.addBindValue(i);
    q.addBindValue(QString("Email_%1").arg(i));
    q.addBindValue(QString("Name_%1").arg(i));
    q.addBindValue(QString("Tag_%1").arg(i));

    if (!q.exec()) {
      qFatal("Insert failed: %s",
             qPrintable(q.lastError().text()));
    }
  }
}

static void BM_FindUserByEmail_WithoutCQRS_Mock(benchmark::State& state) {
  LOG_INFO(1);
  MockIdGenerator generator;
  MockCache cache;
  cache.get_should_fail = true;
  LOG_INFO(2);
  MockDatabase database;
  FakeSqlExecutor executor;
  LOG_INFO(3);
  GenericRepository repository(database, &executor, cache, &generator);
  TestAuthManager manager(repository);
  std::string email = "test_email@gmail.com";
  database.mock_query.next_should_fail = true;
  database.exec_should_fail = true;
  executor.shouldFail = true;
  executor.mock_query.next_should_fail = true;
  LOG_INFO(4);
  for (auto _ : state) {
    LOG_INFO(55);
    auto res = manager.findUserByEmail(email);
    benchmark::DoNotOptimize(res);
  }
}

static void BM_FindUserByEmail_WithoutCQRS_SQLite(benchmark::State& state) {
  MockIdGenerator generator;
  MockCache cache;
  cache.get_should_fail = true;
  auto sqlite = prepareDatabase();
  createSchema(sqlite);
  seedData(sqlite);

  SQLiteDatabase database(sqlite);
  FakeSqlExecutor executor;
  GenericRepository repository(database, &executor, cache, &generator);
  TestAuthManager manager(repository);
  std::string email = "test_email@gmail.com";

  manager.findUserByEmail(email); //warm-up

  for (auto _ : state) {
    auto res = manager.findUserByEmail(email);
    benchmark::DoNotOptimize(res);
  }
}



int main(int argc, char** argv) {
  QCoreApplication app(argc, argv);

  //BENCHMARK(BM_FindUserByEmail_WithoutCQRS_Mock);
  //BENCHMARK(BM_FindUserByEmail_WithoutCQRS_SQLite); //a lot of memory needed

  //BENCHMARK(BM_SaveUser_WithoutCQRS_Mock);
  //BENCHMARK(BM_SaveUser_WithoutCQRS_SQlite);

  // Run all benchmarks
  benchmark::RunSpecifiedBenchmarks();

  return 0;
}


