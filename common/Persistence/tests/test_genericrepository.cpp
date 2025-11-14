#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>

#include "GenericRepository.h"
#include "entities/Chat.h"
#include "entities/ChatMember.h"
#include "entities/Message.h"
#include "entities/MessageStatus.h"
#include "entities/User.h"
#include "entities/UserCredentials.h"
#include "mocks/FakeSqlExecutor.h"
#include "mocks/MockCache.h"
#include "mocks/MockDatabase.h"

TEST_CASE("Test saving entity in database") {
  MockDatabase      db;
  MockCache         cache;
  FakeSqlExecutor   executor;
  GenericRepository rep(db, executor, cache);

  User user;
  user.email    = "romanlobach@gmail.com";
  user.tag      = "roma222";
  user.username = "roma";
  user.id       = 0;

  SECTION("Save user expected executor call") {
    int before = executor.execute_calls;
    rep.save(user);
    REQUIRE(executor.execute_calls == before + 1);
  }

  SECTION("Save user expected returned true status") {
    bool ok = rep.save(user);
    REQUIRE(ok == true);
  }

  SECTION("Save user expected returned mocked id") {
    executor.mocked_id = 6;
    user.id            = 0;
    rep.save(user);
    REQUIRE(user.id == 6);
  }

  MessageStatus message_status;
  message_status.receiver_id = 3;
  message_status.message_id  = 4;
  message_status.is_read     = true;
  auto timepoint             = QDateTime::currentMSecsSinceEpoch();
  message_status.read_at     = timepoint;

  SECTION("Save message_status expected true") { REQUIRE(rep.save(message_status)); }

  SECTION("Save message_status expected any fields is changed") {
    REQUIRE(message_status.receiver_id == 3);
    REQUIRE(message_status.message_id == 4);
    REQUIRE(message_status.is_read == true);
    REQUIRE(message_status.read_at == timepoint);
  }

  SECTION("Save message with id expected id not changed") {
    Message message;
    message.id = 10;
    rep.save(message);
    REQUIRE(message.id == 10);
  }

  SECTION("Save message with id expected don't call executedReturnID") {
    Message message;
    message.id       = 10;
    int before_calls = executor.execute_returning_id_calls;
    rep.save(message);

    REQUIRE(executor.execute_returning_id_calls == before_calls);
  }

  SECTION("Save message without id expected call executedReturnID") {
    Message message;
    int     before_calls = executor.execute_returning_id_calls;
    rep.save(message);

    REQUIRE(executor.execute_returning_id_calls == before_calls + 1);
  }
}

TEST_CASE("For every entity creates valid sql command") {
  MockDatabase      db;
  MockCache         cache;
  FakeSqlExecutor   executor;
  GenericRepository rep(db, executor, cache);

  SECTION("Save user without id expected right created sql command") {
    QString valid_sql =
        "INSERT OR REPLACE INTO users (username, tag, email) "
        "VALUES (?, ?, ?) RETURNING id";
    User user;
    rep.save(user);
    LOG_INFO("Last sql {}", executor.lastSql.toStdString());
    REQUIRE(executor.lastSql == valid_sql);
  }

  SECTION("Save user with id expected right created sql command") {
    QString valid_sql =
        "INSERT OR REPLACE INTO users (id, username, tag, email) "
        "VALUES (?, ?, ?, ?)";
    User user;
    user.id = 1;
    rep.save(user);
    LOG_INFO("Last sql {}", executor.lastSql.toStdString());
    REQUIRE(executor.lastSql == valid_sql);
  }

  SECTION("Save message_status expected right created sql command") {
    QString valid_sql =
        "INSERT OR REPLACE INTO messages_status (message_id, receiver_id, is_read, read_at) "
        "VALUES (?, ?, ?, ?)";
    MessageStatus message_status;
    rep.save(message_status);
    REQUIRE(executor.lastSql == valid_sql);
  }

  SECTION("Save message without id expected right created sql command") {
    QString valid_sql =
        "INSERT OR REPLACE INTO messages (sender_id, chat_id, text, timestamp, local_id) "
        "VALUES (?, ?, ?, ?, ?) RETURNING id";
    Message message;
    rep.save(message);
    REQUIRE(executor.lastSql == valid_sql);
  }

  SECTION("Save message with id expected right created sql command") {
    QString valid_sql =
        "INSERT OR REPLACE INTO messages (id, sender_id, chat_id, text, timestamp, local_id) "
        "VALUES (?, ?, ?, ?, ?, ?)";
    Message message;
    message.id = 10;
    rep.save(message);
    LOG_INFO("Last sql {}", executor.lastSql.toStdString());
    REQUIRE(executor.lastSql == valid_sql);
  }

  SECTION("Save chat without id expected right created sql command") {
    QString valid_sql =
        "INSERT OR REPLACE INTO chats (is_group, name, avatar, created_at) "
        "VALUES (?, ?, ?, ?) RETURNING id";

    Chat chat;
    rep.save(chat);
    REQUIRE(executor.lastSql == valid_sql);
  }

  SECTION("Save chat with id expected right created sql command") {
    QString valid_sql =
        "INSERT OR REPLACE INTO chats (id, is_group, name, avatar, created_at) "
        "VALUES (?, ?, ?, ?, ?)";

    Chat chat;
    chat.id = 10;
    rep.save(chat);
    LOG_INFO("Last sql {}", executor.lastSql.toStdString());
    REQUIRE(executor.lastSql == valid_sql);
  }

  SECTION("Save chat_member expected right created sql command") {
    QString valid_sql =
        "INSERT OR REPLACE INTO chat_members (chat_id, user_id, status, added_at) "
        "VALUES (?, ?, ?, ?)";

    ChatMember chat_member;
    rep.save(chat_member);
    REQUIRE(executor.lastSql == valid_sql);
  }

  SECTION("Save user_credentilas expected right created sql command") {
    QString valid_sql =
        "INSERT OR REPLACE INTO credentials (user_id, hash_password) "
        "VALUES (?, ?)";

    UserCredentials user_credentials;
    rep.save(user_credentials);
    REQUIRE(executor.lastSql == valid_sql);
  }
}

TEST_CASE("Test integration with cache while saving") {
  MockDatabase      db;
  MockCache         cache;
  FakeSqlExecutor   executor;
  GenericRepository rep(db, executor, cache);

  SECTION("Saved entity eepected updated cache") {
    User user;
    user.id               = 10;
    std::string tableKey  = "table_generation:users";
    std::string entityKey = "entity_cache:users:10";

    int before_table  = cache.getCalls(tableKey);
    int before_entity = cache.getCalls(entityKey);

    rep.save(user);

    REQUIRE(cache.getCalls(tableKey) == before_table + 1);
    REQUIRE(cache.getCalls(entityKey) == before_entity + 1);

    auto json = cache.get(entityKey);
    REQUIRE(nlohmann::json(user) == json);
  }

  SECTION("Saved message_status with custom key exepected updated cache") {
    MessageStatus status;
    status.message_id     = 1;
    status.receiver_id    = 2;
    std::string tableKey  = "table_generation:messages_status";
    std::string entityKey = "entity_cache:messages_status:1";

    int before_table  = cache.getCalls(tableKey);
    int before_entity = cache.getCalls(entityKey);

    rep.save(status);

    REQUIRE(cache.getCalls(tableKey) == before_table + 1);
    REQUIRE(cache.getCalls(entityKey) == before_entity + 1);

    auto json = cache.get(entityKey);
    REQUIRE(nlohmann::json(status) == json);
  }

  SECTION("Saved user_credentials with custom key exepected updated cache") {
    UserCredentials credentials;
    credentials.user_id       = 3;
    credentials.hash_password = "123";
    std::string tableKey      = "table_generation:credentials";
    std::string entityKey     = "entity_cache:credentials:3";

    int before_table  = cache.getCalls(tableKey);
    int before_entity = cache.getCalls(entityKey);

    rep.save(credentials);

    REQUIRE(cache.getCalls(tableKey) == before_table + 1);
    REQUIRE(cache.getCalls(entityKey) == before_entity + 1);

    auto json = cache.get(entityKey);
    REQUIRE(nlohmann::json(credentials) == json);
  }

  SECTION("Saved chat_members with custom key exepected updated cache") {
    ChatMember member;
    member.chat_id        = 3;
    member.user_id        = 4;
    std::string tableKey  = "table_generation:chat_members";
    std::string entityKey = "entity_cache:chat_members:3, 4";

    int before_table  = cache.getCalls(tableKey);
    int before_entity = cache.getCalls(entityKey);

    rep.save(member);

    REQUIRE(cache.getCalls(tableKey) == before_table + 1);
    REQUIRE(cache.getCalls(entityKey) == before_entity + 1);

    auto json = cache.get(entityKey);
    REQUIRE(nlohmann::json(member) == json);
  }

  SECTION("Clear cache expected clear all data") {
    ChatMember member;
    member.chat_id        = 3;
    member.user_id        = 4;
    std::string entityKey = "entity_cache:chat_members:3, 4";
    rep.save(member);
    REQUIRE(cache.exists(entityKey));

    rep.clearCache();

    REQUIRE_FALSE(cache.exists(entityKey));
  }
}
