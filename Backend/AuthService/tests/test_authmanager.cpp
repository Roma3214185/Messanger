#include <catch2/catch_all.hpp>

#include "authservice/authmanager.h"
#include "GenericRepository.h"
#include "mocks/FakeSqlExecutor.h"
#include "mocks/MockCache.h"
#include "mocks/MockTheadPool.h"
#include "mocks/MockDatabase.h"
#include "entities/UserCredentials.h"
#include "mocks/MockIdGenerator.h"

struct TestAuthManager : public AuthManager {
    using AuthManager::AuthManager;

    OptionalUser mock_user_by_email;
    OptionalUser mock_user_by_tag;
    std::optional<UserCredentials> mock_credentials;
    bool should_fail = false;

    int last_user_id;
    std::string last_email;
    std::string last_password_to_check;
    std::string last_hash_password;

    std::string last_raw_password;
    std::string mock_hash_password;
    std::string last_tag;

    OptionalUser findUserByEmail(const std::string& email) override {
      last_email = email;
      return mock_user_by_email;
    }

    std::optional<UserCredentials> findUserCredentials(int user_id) override {
      last_user_id = user_id;
      return mock_credentials;
    }

    bool passwordIsValid(const std::string& password_to_check, const std::string& hash_password) override {
      last_password_to_check = password_to_check;
      last_hash_password = hash_password;
      return !should_fail;
    }

    std::string getHashPassword(const std::string& raw_passport) override {
      last_raw_password = raw_passport;
      return mock_hash_password;
    }

    std::optional<User> findUserWithSameTag(const std::string& tag) override {
      last_tag = tag;
      return mock_user_by_tag;
    }
};

struct TestAuthManagerFixture {
    MockCache cache;
    MockDatabase db;
    MockThreadPool pool;
    FakeSqlExecutor executor;
    int user_id = 12;
    User user;
    UserCredentials user_credentials;
    MockIdGenerator generator;
    GenericRepository rep;
    TestAuthManager manager;

    std::string password = "test_password";
    std::string email = "test_email";
    std::string tag = "test_tag";
    std::string username = "test_username";
    std::string avatar = "test_path_to_avatar";
    std::string hash_password = "secret-hash-password-123";

    TestAuthManagerFixture()
        : rep(db, &executor, cache, &generator, &pool)
        , manager(rep) {
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

TEST_CASE("Get user expected create valid sql") {
  TestAuthManagerFixture fix;
  int before_execute_calls = fix.executor.execute_calls;
  std::string expected_sql = "SELECT * FROM users WHERE id = ? LIMIT 1";

  fix.manager.getUser(fix.user_id);

  REQUIRE(fix.executor.lastSql.toStdString() == expected_sql);
  REQUIRE(fix.executor.lastValues.size() == 1);
  REQUIRE(fix.executor.lastValues[0] == fix.user_id);
}

TEST_CASE("Test AUthManager::login") {
  TestAuthManagerFixture fix;
  LoginRequest log_req{.email = fix.email, .password = fix.password};
  fix.manager.mock_user_by_email = fix.user;
  fix.manager.mock_credentials = fix.user_credentials;

  SECTION("User with same email not found expected expected return std::nullopt") {
    fix.manager.mock_user_by_email = std::nullopt;
    auto res = fix.manager.loginUser(log_req);
    REQUIRE(fix.manager.last_email == fix.email);
    REQUIRE(res == std::nullopt);
  }


  SECTION("UserCredentials not found expected expected return std::nullopt") {
    fix.manager.mock_credentials = std::nullopt;
    auto res = fix.manager.loginUser(log_req);
    REQUIRE(fix.manager.last_user_id == fix.user_id);
    REQUIRE(res == std::nullopt);
  }

  SECTION("UserCredentials not valid expected return std::nullopt") {
    fix.manager.should_fail = true;
    auto res = fix.manager.loginUser(log_req);
    REQUIRE(fix.manager.last_hash_password == fix.manager.mock_credentials->hash_password);
    REQUIRE(fix.manager.last_password_to_check == fix.password);
    REQUIRE(res == std::nullopt);
  }

  SECTION("All okay expected return valid user") {
    auto res = fix.manager.loginUser(log_req);
    CHECK(res->id == fix.user.id);
    CHECK(res->avatar == fix.user.avatar);
    CHECK(res->username == fix.user.username);
    CHECK(res->tag == fix.user.tag);
    CHECK(res->email == fix.user.email);
  }
}

TEST_CASE("Test AuthManager::register") {
   TestAuthManagerFixture fix;
   RegisterRequest reg_req{.email = fix.email, .password = fix.password, .name = fix.username, .tag = fix.tag};
   fix.manager.mock_user_by_email = std::nullopt;
    fix.manager.mock_user_by_tag = std::nullopt;

   SECTION("User with same email exist expected return std::nullopt") {
    fix.manager.mock_user_by_email = fix.user;

    auto res = fix.manager.registerUser(reg_req);

    REQUIRE(fix.manager.last_email == reg_req.email);
    REQUIRE(res == std::nullopt);
   }

   SECTION("User with same tag exist expected return std::nullopt") {
     fix.manager.mock_user_by_tag = fix.user;

     auto res = fix.manager.registerUser(reg_req);

     REQUIRE(fix.manager.last_tag == reg_req.tag);
     REQUIRE(res == std::nullopt);
   }

   SECTION("Save fails expected return std::nullopt") {
     fix.executor.shouldFail = true;
     fix.generator.mocked_id = 13424;
     std::string expected_sql = "INSERT OR REPLACE INTO users (id, username, tag, email) VALUES (?, ?, ?, ?)";
     auto res = fix.manager.registerUser(reg_req);

     REQUIRE(fix.executor.lastSql.toStdString() == expected_sql);
     REQUIRE(fix.executor.lastValues.size() == 4);
     CHECK(fix.executor.lastValues[0].toInt() == fix.generator.mocked_id);
     CHECK(fix.executor.lastValues[1].toString().toStdString() == reg_req.name);
     CHECK(fix.executor.lastValues[2].toString().toStdString() == reg_req.tag);
     CHECK(fix.executor.lastValues[3].toString().toStdString() == reg_req.email);
     REQUIRE(res == std::nullopt);
   }

   SECTION("Save fails axpected return std::nullopt") {
     fix.manager.mock_hash_password = fix.hash_password;
     std::string expected_sql = "INSERT OR REPLACE INTO credentials (user_id, hash_password) VALUES (?, ?)";
     auto res = fix.manager.registerUser(reg_req);

     REQUIRE(fix.executor.lastSql.toStdString() == expected_sql);
     REQUIRE(fix.executor.lastValues.size() == 2);
     REQUIRE(res != std::nullopt);
     CHECK(fix.executor.lastValues[0].toInt() == fix.user.id);
     CHECK(fix.executor.lastValues[1].toString().toStdString() == fix.hash_password);

     CHECK(res->id == fix.user.id);
     //CHECK(res->avatar == fix.user.avatar);
     CHECK(res->username == fix.user.username);
     CHECK(res->tag == fix.user.tag);
     CHECK(res->email == fix.user.email);
   }
}

TEST_CASE("Test AuthManager::FingUsersByTag") {
  TestAuthManagerFixture fix;
  int before_execute_calls = fix.executor.execute_calls;
  std::string expected_sql0 = "SELECT * FROM users WHERE tag = ?";
  std::string tag = "test_tag_229";

  fix.manager.findUsersByTag(tag);

  REQUIRE(fix.executor.execute_calls == before_execute_calls + 1);
  REQUIRE(fix.executor.lastSql.toStdString() == expected_sql0);
  REQUIRE(fix.executor.lastValues.size() == 1);
  REQUIRE(fix.executor.lastValues[0].toString().toStdString() == tag);
}
