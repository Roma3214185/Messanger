#include <catch2/catch_all.hpp>

#include <QSqlQuery>
#include "SQLiteDataBase.h"

struct TestSqliteDatabase : public SQLiteDatabase {
    using SQLiteDatabase::SQLiteDatabase;
    using SQLiteDatabase::tableExists;
    using SQLiteDatabase::executeSql;
    using SQLiteDatabase::deleteTable;
    using SQLiteDatabase::initializeSchema;
};

TEST_CASE("Test sqlitedatabase create tables") {
  TestSqliteDatabase db_(":memory:");
  QSqlDatabase database_ = db_.getThreadDatabase();

  SECTION("Execute sql expected false on invalid SQL") {
    bool result = db_.executeSql(database_,
                                  "CREATE TABL invalid_sql");
    REQUIRE_FALSE(result);
  }

  SECTION("Execute valid sql expected true and table exist") {
    bool result = db_.executeSql(database_,
                                  "CREATE TABLE IF NOT EXISTS test_table (id INTEGER PRIMARY KEY)");
    REQUIRE(result);
    REQUIRE(db_.tableExists(database_, "test_table"));
  }

  SECTION("Delete existing table expect no table") {
    bool result = db_.executeSql(database_,
                               "CREATE TABLE IF NOT EXISTS test_table (id INTEGER PRIMARY KEY)");
    REQUIRE(result);

    db_.deleteTable(database_, "test_table");
    REQUIRE_FALSE(db_.tableExists(database_, "test_table"));
  }
}
