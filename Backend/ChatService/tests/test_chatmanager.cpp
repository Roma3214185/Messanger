#include <catch2/catch_all.hpp>

#include "GenericRepository.h"
#include "chatservice/chatmanager.h"
#include "mocks/FakeSqlExecutor.h"
#include "mocks/MockCache.h"
#include "mocks/MockDatabase.h"
#include "mocks/MockIdGenerator.h"
#include "mocks/MockTheadPool.h"

namespace TestChatManager {

struct TestFixture {
  FakeSqlExecutor executor;
  MockCache cache;
  MockThreadPool pool;
  MockDatabase db;
  GenericRepository repository;
  ChatManager manager;
  MockIdGenerator generator;

  TestFixture() : repository(&executor, cache, &pool), manager(&repository, &generator) {}
};

}  // namespace TestChatManager

TEST_CASE("Test chatManager::createPrivateChat") {
  TestChatManager::TestFixture fix;

  SECTION("Given same id's return std::nullopt") {
    auto res = fix.manager.createPrivateChat(4, 4);
    REQUIRE(res == std::nullopt);
  }

  SECTION("Expected 1 call to executor") {
    int before_calls = fix.executor.execute_calls;
    auto res = fix.manager.createPrivateChat(4, 5);
    REQUIRE(fix.executor.execute_calls == before_calls + 1);
  }

  SECTION(
      "Private chat is already created expected 1 call to exsecute and "
      "returned this chat") {
    int before_calls = fix.executor.execute_calls;

    auto res = fix.manager.createPrivateChat(4, 5);
    REQUIRE(fix.executor.execute_calls == before_calls + 1);
  }
}

TEST_CASE("Test chatManager::getMembersOfChat") {
  TestChatManager::TestFixture fix;

  SECTION("Expected creating valid sql") {
    int chat_id = 1234;
    std::string expected_sql = "SELECT * FROM chat_members WHERE chat_id = ?";
    fix.executor.lastSql.clear();

    fix.manager.getMembersOfChat(chat_id);

    REQUIRE(fix.executor.lastSql.toStdString() == expected_sql);
    REQUIRE(fix.executor.lastValues.size() == 1);
    REQUIRE(fix.executor.lastValues[0] == chat_id);
  }
}

TEST_CASE("Test chatManager::getChatsOfUserId") {
  TestChatManager::TestFixture fix;

  SECTION("Expected creating valid sql") {
    int member_id = 12342;
    std::string expected_sql = "SELECT * FROM chat_members WHERE user_id = ?";
    fix.executor.lastSql.clear();

    fix.manager.getChatsIdOfUser(member_id);

    REQUIRE(fix.executor.lastSql.toStdString() == expected_sql);
    REQUIRE(fix.executor.lastValues.size() == 1);
    REQUIRE(fix.executor.lastValues[0] == member_id);
  }
}

TEST_CASE("Test chatManager::getMembersCount") {
  TestChatManager::TestFixture fix;

  SECTION("Expected creating valid sql") {
    int chat_id = 12;
    std::string expected_sql = "SELECT * FROM chat_members WHERE chat_id = ?";
    fix.executor.lastSql.clear();

    fix.manager.getMembersCount(chat_id);

    REQUIRE(fix.executor.lastSql.toStdString() == expected_sql);
    REQUIRE(fix.executor.lastValues.size() == 1);
    REQUIRE(fix.executor.lastValues[0] == chat_id);
  }
}

TEST_CASE("Test chatManager::getOtherMemberId") {
  TestChatManager::TestFixture fix;

  SECTION("Expected creating valid sql") {
    int member_id = 12;
    int chat_id = 11;
    std::string expected_sql = "SELECT * FROM chat_members WHERE chat_id = ?";
    fix.executor.lastSql.clear();

    fix.manager.getOtherMemberId(chat_id, member_id);

    REQUIRE(fix.executor.lastSql.toStdString() == expected_sql);
    REQUIRE(fix.executor.lastValues.size() == 1);
    REQUIRE(fix.executor.lastValues[0] == chat_id);
  }
}

TEST_CASE("Test chatManager::getChatById") {
  TestChatManager::TestFixture fix;

  SECTION("Expected creating valid sql") {
    int chat_id = 9;
    std::string expected_sql = "SELECT * FROM chats WHERE id = ? LIMIT 1";
    fix.executor.lastSql.clear();

    fix.manager.getChatById(chat_id);

    REQUIRE(fix.executor.lastSql.toStdString() == expected_sql);
    REQUIRE(fix.executor.lastValues.size() == 1);
    REQUIRE(fix.executor.lastValues[0] == chat_id);
  }
}
