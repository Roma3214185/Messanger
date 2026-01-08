#include <catch2/catch_all.hpp>

#include "GenericRepository.h"
#include "authservice/authmanager.h"
#include "entities/UserCredentials.h"
#include "mocks/FakeSqlExecutor.h"
#include "mocks/MockCache.h"
#include "mocks/MockDatabase.h"
#include "mocks/MockIdGenerator.h"
#include "mocks/MockTheadPool.h"

struct TestProtectedAuthManager : public AuthManager {
  using AuthManager::AuthManager;
  using AuthManager::findUserByEmail;
  using AuthManager::findUserCredentials;
  using AuthManager::findUserWithSameTag;
};

struct TestAuthManagerProtectedFixture {
  MockCache cache;
  MockDatabase db;
  MockThreadPool pool;
  FakeSqlExecutor executor;
  int user_id = 12;
  User user;
  UserCredentials user_credentials;
  MockIdGenerator generator;
  GenericRepository rep;
  TestProtectedAuthManager manager;

  std::string password = "test_password";
  std::string email = "test_email";
  std::string tag = "test_tag";
  std::string username = "test_username";
  std::string avatar = "test_path_to_avatar";
  std::string hash_password = "secret-hash-password-123";

  TestAuthManagerProtectedFixture() : rep(&executor, cache, &pool), manager(rep, &generator) {
    user.id = user_id;
    user.email = email;
    user.tag = tag;
    user.username = username;
    user.avatar = avatar;

    user_credentials.user_id = user_id;
    user_credentials.hash_password = hash_password;
    generator.mocked_id = user_id;
  }
};

TEST_CASE("Test findUserByEmail") {
  TestAuthManagerProtectedFixture fix;

  int before_execute_calls = fix.executor.execute_calls;
  std::string expected_sql = "SELECT * FROM users WHERE email = ?";

  fix.manager.findUserByEmail(fix.email);

  REQUIRE(fix.executor.execute_calls == before_execute_calls + 1);
  REQUIRE(fix.executor.lastSql.toStdString() == expected_sql);
  REQUIRE(fix.executor.lastValues.size() == 1);
  REQUIRE(fix.executor.lastValues[0].toString() == fix.email);
}

TEST_CASE("Test findUserByTag") {
  TestAuthManagerProtectedFixture fix;

  int before_execute_calls = fix.executor.execute_calls;
  std::string expected_sql = "SELECT * FROM users WHERE tag = ?";

  fix.manager.findUserWithSameTag(fix.tag);

  REQUIRE(fix.executor.execute_calls == before_execute_calls + 1);
  REQUIRE(fix.executor.lastSql.toStdString() == expected_sql);
  REQUIRE(fix.executor.lastValues.size() == 1);
  REQUIRE(fix.executor.lastValues[0].toString() == fix.tag);
}

TEST_CASE("Test findUserCredentials") {
  TestAuthManagerProtectedFixture fix;

  int before_execute_calls = fix.executor.execute_calls;
  std::string expected_sql = "SELECT * FROM credentials WHERE user_id = ?";

  fix.manager.findUserCredentials(fix.user.id);

  REQUIRE(fix.executor.execute_calls == before_execute_calls + 1);
  REQUIRE(fix.executor.lastSql.toStdString() == expected_sql);
  REQUIRE(fix.executor.lastValues.size() == 1);
  REQUIRE(fix.executor.lastValues[0].toInt() == fix.user.id);
}
