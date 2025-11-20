#include <catch2/catch_all.hpp>

#include "authservice/authmanager.h"
#include "GenericRepository.h"
#include "mocks/FakeSqlExecutor.h"
#include "mocks/MockCache.h"
#include "mocks/MockTheadPool.h"
#include "mocks/MockDatabase.h"

// ISqlExecutor*  executor_;
// ICacheService& cache_;
// IThreadPool*    pool_;
// IDataBase&     database_;

struct TestAuthManagerFixture {
    MockCache cache;
    MockDatabase db;
    MockThreadPool pool;
    FakeSqlExecutor executor;
    int user_id = 12;

    GenericRepository rep;
    AuthManager manager;

    TestAuthManagerFixture()
        : rep(db, &executor, cache, &pool)
        , manager(rep) {}
};

TEST_CASE("Get user expected create valid sql") {
  TestAuthManagerFixture fix;
  int before_execute_calls = fix.executor.execute_calls;
  std::string expected_sql = "SELECT * FROM users WHERE id = ? LIMIT 1";

  fix.manager.getUser(fix.user_id);

  REQUIRE(fix.executor.lastSql.toStdString() == expected_sql);
  REQUIRE(fix.executor.lastValues.size() == 1);
  REQUIRE(fix.executor.lastValues[0] == fix.user_id);
}

TEST_CASE("LoginUser expected create valid sql for find user with this email") {
  TestAuthManagerFixture fix;
  int before_execute_calls = fix.executor.execute_calls;
  std::string expected_sql1 = "SELECT * FROM users WHERE email = ?";
  LoginRequest log_req{.email = "test_email", .password = "test_password"};
  fix.executor.last_sqls.clear();

  fix.manager.loginUser(log_req);

  auto last_sqls = fix.executor.last_sqls;
  REQUIRE(last_sqls.size() == 1);
  REQUIRE(last_sqls[0] == expected_sql1);
}

TEST_CASE("RegisterUser expected create valid sqls") {
  TestAuthManagerFixture fix;
  int before_execute_calls = fix.executor.execute_calls;
  std::string expected_sql0 = "SELECT * FROM users WHERE email = ?";
  std::string expected_sql1 = "SELECT * FROM users WHERE tag = ?";
  std::string expected_sql2 = "INSERT OR REPLACE INTO users (username, tag, email) VALUES (?, ?, ?) RETURNING id";
  std::string expected_sql3 = "INSERT OR REPLACE INTO credentials (user_id, hash_password) VALUES (?, ?)";
  RegisterRequest reg_req{.email = "test_email", .password = "test_password", .name = "test_name", .tag = "test_tag"};
  fix.executor.last_sqls.clear();

  fix.manager.registerUser(reg_req);

  auto last_sqls = fix.executor.last_sqls;
  REQUIRE(last_sqls.size() == 4);
  CHECK(last_sqls[0] == expected_sql0);
  CHECK(last_sqls[1] == expected_sql1);
  CHECK(last_sqls[2] == expected_sql2);
  CHECK(last_sqls[3] == expected_sql3);
}

TEST_CASE("FingUserByTag expected create valid sql") {
  TestAuthManagerFixture fix;
  int before_execute_calls = fix.executor.execute_calls;
  std::string expected_sql0 = "SELECT * FROM users WHERE tag = ?";
  std::string tag = "test_tag_229";

  fix.manager.findUserByTag(tag);

  REQUIRE(fix.executor.execute_calls == before_execute_calls + 1);
  REQUIRE(fix.executor.lastSql.toStdString() == expected_sql0);
  REQUIRE(fix.executor.lastValues.size() == 1);
  REQUIRE(fix.executor.lastValues[0].toString().toStdString() == tag);
}


/*


std::vector<User> AuthManager::findUserByTag(const string& tag) {
  return rep.findByField<User>(UserTable::Tag, QString::fromStdString(tag));
}

AuthManager::AuthManager(GenericRepository& repository) : rep(repository) {}

*/


