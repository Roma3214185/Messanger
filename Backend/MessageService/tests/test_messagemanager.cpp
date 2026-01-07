#include <catch2/catch_all.hpp>

#include "messageservice/dto/GetMessagePack.h"
#include "messageservice/managers/MessageManager.h"
#include "mocks/FakeSqlExecutor.h"
#include "mocks/MockCache.h"
#include "mocks/MockDatabase.h"
#include "mocks/MockIdGenerator.h"
#include "mocks/MockTheadPool.h"

TEST_CASE("Test") {
  MockDatabase db;
  MockThreadPool pool;
  MockCache cache;
  FakeSqlExecutor executor;
  MockIdGenerator generator;
  GenericRepository repository(&executor, cache, &pool);
  MessageManager manager(&repository, &executor, &generator, cache);

  SECTION("getChatMessages expected create valid sql request") {
    cache.clearCache();
    GetMessagePack pack{.chat_id = 1, .limit = 2, .before_id = 3, .user_id = 4};
    int before = executor.execute_calls;

    manager.getChatMessages(pack);

    std::string expected_sql = "SELECT * FROM messages "
                               "JOIN messages_status ON "
                               "id = messages_status.message_id "
                               "WHERE chat_id = ? "
                               "AND messages_status.receiver_id = ? "
                               "AND id < ? ORDER BY timestamp DESC LIMIT 2";
    REQUIRE(executor.execute_calls == before + 1);
    CHECK(executor.lastSql.toStdString() == expected_sql);
    CHECK(executor.lastValues.size() == 3);
    CHECK(executor.lastValues[0] == 1);
    CHECK(executor.lastValues[1] == 4);
    CHECK(executor.lastValues[2] == 3);
  }

  SECTION("getMessagesStatus expected create valid sql request") {
    cache.clearCache();
    std::vector<Message> messages;
    Message message1, message2, message3, message4;
    message1.id = 1; message2.id = 2; message3.id = 3; message4.id = 4;
    messages.push_back(message1);
    messages.push_back(message2);
    messages.push_back(message3);
    messages.push_back(message4);
    int user_id = 5;
    int before = executor.execute_calls;

    manager.getMessagesStatus(messages, user_id);

    std::string expected_sql = "SELECT * FROM messages_status WHERE message_id "
                               "= ? AND receiver_id = ?";
    REQUIRE(executor.execute_calls == before + messages.size());
    CHECK(executor.lastSql.toStdString() == expected_sql);
    CHECK(executor.lastValues.size() == 2);
    CHECK(executor.lastValues[0] == 4);
    CHECK(executor.lastValues[1] == user_id);
  }

  SECTION("Get message expected create right sql") {
    int before_calls = executor.execute_calls;
    int message_id = 3;
    manager.getMessage(message_id);

    std::string expected_sql = "SELECT * FROM messages WHERE id = ? LIMIT 1";
    REQUIRE(executor.execute_calls == before_calls + 1);
    CHECK(executor.lastSql.toStdString() == expected_sql);
    CHECK(executor.lastValues.size() == 1);
    CHECK(executor.lastValues[0] == message_id);
  }

  // SECTION("Save message status expected create right sql") {
  //   MessageStatus to_save;
  //   to_save.message_id  = 1;
  //   to_save.receiver_id = 2;
  //   to_save.is_read     = true;
  //   to_save.read_at     = 123;
  //   int before_execute  = executor.execute_calls;

  //   auto res = manager.saveMessageStatus(to_save);

  //   std::string expected_sql =
  //       "INSERT OR REPLACE INTO messages_status (message_id, receiver_id,
  //       is_read, read_at) " "VALUES (?, ?, ?, ?)";

  //   CHECK(db.last_prepared_sql == expected_sql);
  // }

  SECTION("Get message status expected create right sql") {
    int before_execute = executor.execute_calls;
    int message_id = 23;
    int receiver_id = 156;

    manager.getMessageStatus(message_id, receiver_id);

    std::string expected_sql = "SELECT * FROM messages_status WHERE message_id "
                               "= ? AND receiver_id = ? LIMIT 1";

    REQUIRE(executor.execute_calls == before_execute + 1);
    CHECK(executor.lastSql.toStdString() == expected_sql);

    CHECK(executor.lastValues.size() == 2);
    CHECK(executor.lastValues[0] == message_id);
    CHECK(executor.lastValues[1] == receiver_id);
  }
}
