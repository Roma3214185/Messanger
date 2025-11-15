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

    manager.getChatMessages(pack);

    std::string expected_sql = "SELECT * FROM messages "
                          "JOIN messages_status ON "
                          "id = messages_status.message_id "
                          "WHERE chat_id = ? "
                          "AND messages_status.user_id = ? "
                          "AND id < ? ORDER BY timestamp DESC LIMIT 2";
    REQUIRE(executor.lastSql.toStdString() == expected_sql);
  }


}
