#include <crow.h>

#include <QNetworkReply>
#include <catch2/catch_all.hpp>

#include "GenericRepository.h"
#include "interfaces/ICacheService.h"
#include "interfaces/IDataBase.h"
#include "interfaces/ISqlExecutor.h"
#include "messageservice/controller.h"
#include "messageservice/managers/MessageManager.h"
#include "mocks.h"
#include "mocks/messageservice/SecondTestController.h"
#include "mocks/messageservice/TestController.h"

struct SharedFixture {
  MockRabitMQClient rabit_client;
  MockDatabase db;
  FakeSqlExecutor executor;
  MockCache cache;
  Routes mock_routes;
  MockConfigProvider provider;
  GenericRepository rep;
  MessageManager manager;
  MockThreadPool pool;
  MockIdGenerator generator;

  SharedFixture()
      : mock_routes(MockUtils::getMockRoutes()),
        provider(),
        rep(&executor, cache, &pool),
        manager(&rep, &executor, &generator) {
    provider.mock_routes = mock_routes;
  }
};

TEST_CASE("Test cotroller works with rabitMQ") {
  SharedFixture fix;
  TestController controller(&fix.rabit_client, &fix.manager, &fix.pool, &fix.provider);

  SECTION("Subscrive on message to save expected valid data") {
    int before = fix.rabit_client.subscribe_cnt;
    controller.subscribeToSaveMessage();

    REQUIRE(fix.rabit_client.subscribe_cnt == before + 1);
    auto last_subscribe_data = fix.rabit_client.last_subscribe_request;
    CHECK(last_subscribe_data.exchange == fix.mock_routes.exchange);
    CHECK(last_subscribe_data.queue == fix.mock_routes.saveMessageQueue);
    CHECK(last_subscribe_data.exchange_type == fix.mock_routes.exchangeType);
    CHECK(last_subscribe_data.routing_key == fix.mock_routes.saveMessage);
  }

  SECTION("Subscrive on message to save expected call valid callback function") {
    Message test_message;
    int before_subscribe_call = fix.rabit_client.subscribe_cnt;
    controller.subscribeToSaveMessage();

    REQUIRE(fix.rabit_client.subscribe_cnt == before_subscribe_call + 1);
    int before_callback_call = controller.call_save_message;

    fix.rabit_client.callLastCallback(nlohmann::json(test_message).dump());
    REQUIRE(controller.call_save_message == before_callback_call + 1);
    REQUIRE(controller.last_payload == nlohmann::json(test_message).dump());
  }

  SECTION("Subscrive on message_status to save expected valid data") {
    int before = fix.rabit_client.subscribe_cnt;
    controller.subscribeToSaveMessageStatus();

    REQUIRE(fix.rabit_client.subscribe_cnt == before + 1);
    auto last_subscribe_data = fix.rabit_client.last_subscribe_request;
    CHECK(last_subscribe_data.exchange == fix.mock_routes.exchange);
    CHECK(last_subscribe_data.queue == fix.mock_routes.saveMessageStatusQueue);
    CHECK(last_subscribe_data.exchange_type == fix.mock_routes.exchangeType);
    CHECK(last_subscribe_data.routing_key == fix.mock_routes.saveMessageStatus);
  }

  SECTION(
      "Subscrive on message_status to save expected call valid callback "
      "function") {
    MessageStatus test_message_status;
    int before_subscribe_call = fix.rabit_client.subscribe_cnt;
    controller.subscribeToSaveMessageStatus();

    REQUIRE(fix.rabit_client.subscribe_cnt == before_subscribe_call + 1);
    int before_callback_call = controller.call_save_message_status;

    fix.rabit_client.callLastCallback(nlohmann::json(test_message_status).dump());
    REQUIRE(controller.call_save_message_status == before_callback_call + 1);
    REQUIRE(controller.last_payload == nlohmann::json(test_message_status).dump());
  }
}

TEST_CASE("Test controller handles saved enitites") {
  SharedFixture fix;
  SecondTestController controller(&fix.rabit_client, &fix.manager, &fix.pool, &fix.provider);

  // SECTION("handleSaveMessage expected call to pool and publish to rabitMQ") {
  //   Message message{.id = 2, .local_id = "121", .sender_id = 12};
  //   int     before_publish_call = fix.rabit_client.publish_cnt;
  //   int     before_pool_cnt     = fix.pool.call_count;

  //   controller.handleSaveMessage(nlohmann::json(message).dump());

  //   REQUIRE(fix.pool.call_count == before_pool_cnt + 1);
  //   REQUIRE(fix.rabit_client.publish_cnt == before_publish_call + 1);

  //   auto last_publish_request = fix.rabit_client.last_publish_request;
  //   REQUIRE(last_publish_request.message == nlohmann::json(message).dump());
  // }

  SECTION(
      "handleSaveMessage receive invalid payload expected no call to pool "
      "and no publish to "
      "rabitMQ") {
    int before_publish_call = fix.rabit_client.publish_cnt;
    int before_pool_cnt = fix.pool.call_count;

    controller.handleSaveMessage("Invalid payload");

    REQUIRE(fix.pool.call_count == before_pool_cnt);
    REQUIRE(fix.rabit_client.publish_cnt == before_publish_call);
  }

  SECTION("handleSaveMessageStatus expected call to pool and publish to rabitMQ") {
    MessageStatus message_status(3, 1234, true, utils::time::getCurrentTime());
    int before_publish_call = fix.rabit_client.publish_cnt;
    int before_pool_cnt = fix.pool.call_count;

    controller.handleSaveMessageStatus(nlohmann::json(message_status).dump());

    REQUIRE(fix.pool.call_count == before_pool_cnt + 1);
    REQUIRE(fix.rabit_client.publish_cnt == before_publish_call + 1);

    auto last_publish_request = fix.rabit_client.last_publish_request;
    REQUIRE(last_publish_request.message == nlohmann::json(message_status).dump());
  }

  SECTION(
      "handleSaveMessageStatus receive invalid payload expected no call to "
      "pool and no publish to "
      "rabitMQ") {
    int before_publish_call = fix.rabit_client.publish_cnt;
    int before_pool_cnt = fix.pool.call_count;

    controller.handleSaveMessageStatus("invalid payload");

    REQUIRE(fix.pool.call_count == before_pool_cnt);
    REQUIRE(fix.rabit_client.publish_cnt == before_publish_call);
  }
}
