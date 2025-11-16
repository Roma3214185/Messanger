#include <catch2/catch_all.hpp>

#include "messageservice/managers/MessageManager.h"
#include "mocks/FakeSqlExecutor.h"
#include "mocks/MockDatabase.h"
#include "mocks/MockCache.h"
#include "mocks/MockTheadPool.h"

TEST_CASE("Test") {
  MockDatabase db;
  MockThreadPool pool;
  MockCache cache;
  FakeSqlExecutor executor;
  GenericRepository repository(db, &executor, cache, &pool);
  MessageManager manager(&repository, &executor, cache);

  SECTION("getChatMessages expected create valid sql request") {
    cache.clearCache();
    GetMessagePack pack{.chat_id = 1, .limit = 2, .before_id = 3, .user_id = 4};
    int before = executor.execute_calls;

    manager.getChatMessages(pack);

    std::string expected_sql = "SELECT * FROM messages "
                          "JOIN messages_status ON "
                          "id = messages_status.message_id "
                          "WHERE chat_id = ? "
                          "AND messages_status.user_id = ? "
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
    messages.push_back(Message{.id = 1});
    messages.push_back(Message{.id = 2});
    messages.push_back(Message{.id = 3});
    messages.push_back(Message{.id = 4});
    int user_id = 5;
    int before = executor.execute_calls;

    manager.getMessagesStatus(messages, user_id);

    std::string expected_sql = "SELECT * FROM messages_status WHERE message_id = ? AND receiver_id = ?";
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

  SECTION("Save message status expected create right sql") {
    MessageStatus to_save;
    to_save.message_id = 1;
    to_save.receiver_id = 2;
    to_save.is_read = true;
    to_save.read_at = 123;
    int before_execute = executor.execute_calls;
    int before_return_id_execute = executor.execute_returning_id_calls;

    manager.saveMessageStatus(to_save);

    std::string expected_sql = "INSERT OR REPLACE INTO messages_status (message_id, receiver_id, is_read, read_at) "
                               "VALUES (?, ?, ?, ?)";

    REQUIRE(executor.execute_calls == before_execute + 1);
    CHECK(executor.lastSql.toStdString() == expected_sql);
    CHECK(executor.execute_returning_id_calls == before_return_id_execute);

    CHECK(executor.lastValues.size() == 4);
    CHECK(executor.lastValues[0] == to_save.message_id);
    CHECK(executor.lastValues[1] == to_save.receiver_id);
    CHECK(executor.lastValues[2] == 1);
    CHECK(executor.lastValues[3] == 123);
  }


}
