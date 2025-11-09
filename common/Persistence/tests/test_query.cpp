#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>

#include "Query.h"
#include "entities/Message.h"
#include "entities/MessageStatus.h"
#include "interfaces/BaseQuery.h"
#include "mocks/FakeSqlExecutor.h"
#include "mocks/MockCache.h"
#include "mocks/MockDatabase.h"

TEST_CASE("Test select query create right sql command") {
  MockCache       cache;
  MockDatabase    database;
  FakeSqlExecutor executor;

  SECTION("Select message by id") {
    auto query = QueryFactory::createSelect<Message>(executor, cache);
    query->where("id", 10);
    QString valid_sql = "SELECT * FROM messages WHERE id = ?";

    query->execute();
    REQUIRE(executor.lastSql == valid_sql);
  }

  SECTION("Select message_status by message_id") {
    auto query = QueryFactory::createSelect<MessageStatus>(executor, cache);
    query->where("message_id", 10);
    QString valid_sql = "SELECT * FROM messages_status WHERE message_id = ?";

    query->execute();
    REQUIRE(executor.lastSql == valid_sql);
  }

  SECTION("Select message with filters") {
    auto query = QueryFactory::createSelect<Message>(executor, cache);
    query->where("id", 10);
    query->where("chat_id", 3);
    query->where("sender_id", 5);
    QString valid_sql = "SELECT * FROM messages WHERE id = ? AND chat_id = ? AND sender_id = ?";

    query->execute();
    REQUIRE(executor.lastSql == valid_sql);
  }

  SECTION("Select message with different filters") {
    auto query = QueryFactory::createSelect<Message>(executor, cache);
    query->where("id", 10);
    query->where("chat_id", 3);
    query->orderBy("timestamp");
    QString valid_sql =
        "SELECT * FROM messages WHERE id = ? AND chat_id = ? ORDER BY timestamp ASC";

    query->execute();
    REQUIRE(executor.lastSql == valid_sql);
  }

  SECTION("Select message with custom filter") {
    auto query = QueryFactory::createSelect<Message>(executor, cache);
    query->where("id", 10);
    query->where("chat_id", "<", 3);
    query->orderBy("timestamp");
    QString valid_sql =
        "SELECT * FROM messages WHERE id = ? AND chat_id < ? ORDER BY timestamp ASC";

    query->execute();
    REQUIRE(executor.lastSql == valid_sql);
  }

  SECTION("Select message with limit") {
    auto query = QueryFactory::createSelect<Message>(executor, cache);
    query->limit(4);
    QString valid_sql = "SELECT * FROM messages LIMIT 4";

    query->execute();
    LOG_INFO("Last sql {}", executor.lastSql.toStdString());
    REQUIRE(executor.lastSql == valid_sql);
  }

  SECTION("2 times select different queue expected not hit the cashe") {
    auto query1 = QueryFactory::createSelect<Message>(executor, cache);
    query1->limit(4);
    auto query2 = QueryFactory::createSelect<Message>(executor, cache);
    query2->limit(5);

    int before_set      = cache.set_calls;
    int before_pipeline = cache.set_pipeline_calls;

    query1->execute();
    query2->execute();

    REQUIRE(cache.set_calls == before_set + 2);
    REQUIRE(cache.set_pipeline_calls == before_pipeline + 2);
  }

  SECTION("2 times select same queue expected hit the cashe") {
    auto query1 = QueryFactory::createSelect<Message>(executor, cache);
    query1->limit(4);
    auto query2 = QueryFactory::createSelect<Message>(executor, cache);
    query2->limit(4);

    int before_set      = cache.set_calls;
    int before_pipeline = cache.set_pipeline_calls;

    query1->execute();
    query2->execute();

    REQUIRE(cache.set_calls == before_set + 1);
    REQUIRE(cache.set_pipeline_calls == before_pipeline + 1);
  }
}

template <typename T>
struct MockSelectedQuery : public SelectQuery<T> {
  using SelectQuery<T>::SelectQuery;

  std::string createCacheKey(QString sql, int generation_hash, int params_hash) const {
    return SelectQuery<T>::createCacheKey(sql, generation_hash, params_hash);
  }
};

TEST_CASE("Test creating keys") {
  MockCache                  cache;
  FakeSqlExecutor            executor;
  MockSelectedQuery<Message> selected_query(executor, cache);

  SECTION("Create valid query key") {
    QString     sql             = "SELECT * FROM users WHERE id = 3";
    int         generation_hash = 12002;
    int         params_hash     = 1222;
    std::string expected_key = "query_cache:SELECT * FROM users WHERE id = 3:gen=12002:params=1222";

    std::string created_key = selected_query.createCacheKey(sql, generation_hash, params_hash);

    REQUIRE(created_key == expected_key);
  }
}
