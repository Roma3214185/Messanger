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
#include "mocks/MockIdGenerator.h"

TEST_CASE("Test saving entity in database") {
  MockQuery query;
  MockDatabase      db;
  db.mock_query = query;
  MockCache         cache;
  FakeSqlExecutor   executor;
  GenericRepository rep(db, &executor, cache);

  int mocked_id = 6;
  User user;
  user.id = mocked_id;
  user.email    = "romanlobach@gmail.com";
  user.tag      = "roma222";
  user.username = "roma";
  user.id       = mocked_id;

  // SECTION("Save user expected executor call") {
  //   int before = executor.execute_calls;
  //   rep.save(user);
  //   REQUIRE(executor.execute_calls == before + 1);
  // }


  SECTION("Save user expected returned true status") {
    bool ok = rep.save(user);
    REQUIRE(ok == true);
  }


  SECTION("Save user with invalid id expected false") {
    user.id = 0;
    REQUIRE_FALSE(rep.save(user));
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
}

// TEST_CASE("For every entity creates valid sql command") {
//   MockDatabase      db;
//   MockCache         cache;
//   FakeSqlExecutor   executor;
//   GenericRepository rep(db, &executor, cache);


//   SECTION("Save user expected right created sql command") {
//     std::string valid_sql =
//         "INSERT OR REPLACE INTO users (id, username, tag, email) "
//         "VALUES (?, ?, ?, ?)";
//     User user;
//     rep.save(user);

//     REQUIRE(db.last_execute_sql == valid_sql);
//   }


//   SECTION("Save message_status expected right created sql command") {
//     std::string valid_sql =
//         "INSERT OR REPLACE INTO messages_status (message_id, receiver_id, is_read, read_at) "
//         "VALUES (?, ?, ?, ?)";
//     MessageStatus message_status;
//     rep.save(message_status);
//     REQUIRE(db.last_execute_sql == valid_sql);
//   }


//   SECTION("Save message expected right created sql command") {
//     QString valid_sql =
//         "INSERT OR REPLACE INTO messages (id, sender_id, chat_id, text, timestamp, local_id) "
//         "VALUES (?, ?, ?, ?, ?, ?)";
//     Message message;
//     message.id = 10;
//     rep.save(message);

//     REQUIRE(db.last_execute_sql == valid_sql);
//   }


//   SECTION("Save chat expected right created sql command") {
//     std::string valid_sql =
//         "INSERT OR REPLACE INTO chats (id, is_group, name, avatar, created_at) "
//         "VALUES (?, ?, ?, ?, ?)";

//     Chat chat;
//     chat.id = 10;
//     rep.save(chat);

//     REQUIRE(db.last_execute_sql == valid_sql);
//   }


//   SECTION("Save chat_member expected right created sql command") {
//     std::string valid_sql =
//         "INSERT OR REPLACE INTO chat_members (chat_id, user_id, status, added_at) "
//         "VALUES (?, ?, ?, ?)";

//     ChatMember chat_member;
//     rep.save(chat_member);
//     REQUIRE(db.last_execute_sql == valid_sql);
//   }


//   SECTION("Save user_credentilas expected right created sql command") {
//     std::string valid_sql =
//         "INSERT OR REPLACE INTO credentials (user_id, hash_password) "
//         "VALUES (?, ?)";

//     UserCredentials user_credentials;
//     rep.save(user_credentials);
//     REQUIRE(db.last_execute_sql == valid_sql);
//   }
// }

TEST_CASE("Test integration with cache while saving") {
  MockDatabase      db;
  MockCache         cache;
  FakeSqlExecutor   executor;
  GenericRepository rep(db, &executor, cache);

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

    auto user_from_cache = cache.get(entityKey);
    REQUIRE(user_from_cache != std::nullopt);
    REQUIRE(nlohmann::json(user).dump() == *user_from_cache);
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

    auto json_from_cache = cache.get(entityKey);
    REQUIRE(json_from_cache != std::nullopt);
    REQUIRE(nlohmann::json(status).dump() == *json_from_cache);
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

    auto json_from_cache = cache.get(entityKey);
    REQUIRE(json_from_cache != std::nullopt);
    REQUIRE(nlohmann::json(credentials).dump() == *json_from_cache);
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
    REQUIRE(json);
    REQUIRE(nlohmann::json(member).dump() == *json);
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
