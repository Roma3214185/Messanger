#include <catch2/catch_all.hpp>

#include "messageservice/controller.h"
#include "mocks/MockRabitMQClient.h"
#include "GenericRepository.h"
#include "interfaces/IDataBase.h"
#include "interfaces/ISqlExecutor.h"
#include "interfaces/ICacheService.h"
#include "messageservice/managers/MessageManager.h"

#include "mocks/MockConfigProvider.h"
#include "mocks/MockRabitMQClient.h"
#include "mocks/MockCache.h"
#include "mocks/FakeSqlExecutor.h"
#include "mocks/MockDatabase.h"

class TestController : public Controller {
  public:
    int call_save_message = 0;
    int call_save_message_status = 0;
    std::string last_payload = "";

    using Controller::subscribeSaveMessage;
    using Controller::subscribeSaveMessageStatus;
    using Controller::Controller;

    void handleSaveMessage(const std::string& payload) override {
      ++call_save_message;
      last_payload = payload;
    }

    void handleSaveMessageStatus(const std::string& payload) override {
      ++call_save_message_status;
      last_payload = payload;
    }
};

TEST_CASE("Test cotroller works with rabitMQ") {
  MockRabitMQClient rabit_client;
  MockDatabase db;
  FakeSqlExecutor executor;
  MockCache cache;

  Routes mock_routes;
  mock_routes.messageSaved       = "test_message_saved";
  mock_routes.saveMessage        = "test_save_message";
  mock_routes.saveMessageStatus  = "test_save_message_status";
  mock_routes.notificationQueue  = "test_notification_service_queue";
  mock_routes.exchange           = "test_app.events";
  mock_routes.messageQueue       = "test_message_service_queue";
  mock_routes.messageStatusSaved = "test_message_status_saved";

  MockConfigProvider provider;
  provider.mock_routes = mock_routes;


  GenericRepository rep(db, executor, cache);
  MessageManager manager(&rep);
  TestController controller(&rabit_client, &manager, &provider);

  SECTION("Subscrive on message to save expected valid data") {
    int before = rabit_client.subscribe_cnt;
    controller.subscribeSaveMessage();

    REQUIRE(rabit_client.subscribe_cnt == before + 1);
    auto last_subscribe_data = rabit_client.last_subscribe_request;
    REQUIRE(last_subscribe_data.exchange == "test_app.events");
    REQUIRE(last_subscribe_data.queue == "test_message_service_queue");
    REQUIRE(last_subscribe_data.exchangeType == "topic");
    REQUIRE(last_subscribe_data.routingKey == "test_save_message");
  }

  SECTION("Subscrive on message to save expected call valid callback function") {
    Message test_message;
    int before_subscribe_call = rabit_client.subscribe_cnt;
    controller.subscribeSaveMessage();

    REQUIRE(rabit_client.subscribe_cnt == before_subscribe_call + 1);
    int before_callback_call = controller.call_save_message;

    rabit_client.callLastCallback(nlohmann::json(test_message).dump());
    REQUIRE(controller.call_save_message == before_callback_call + 1);
    REQUIRE(controller.last_payload == nlohmann::json(test_message).dump());
  }

  SECTION("Subscrive on message_status to save expected valid data") {
    int before = rabit_client.subscribe_cnt;
    controller.subscribeSaveMessageStatus();

    REQUIRE(rabit_client.subscribe_cnt == before + 1);
    auto last_subscribe_data = rabit_client.last_subscribe_request;
    REQUIRE(last_subscribe_data.exchange == "test_app.events");
    REQUIRE(last_subscribe_data.queue == "test_message_service_queue");
    REQUIRE(last_subscribe_data.exchangeType == "topic");
    REQUIRE(last_subscribe_data.routingKey == "test_save_message_status");
  }

  SECTION("Subscrive on message_status to save expected call valid callback function") {
    MessageStatus test_message_status;
    int before_subscribe_call = rabit_client.subscribe_cnt;
    controller.subscribeSaveMessageStatus();

    REQUIRE(rabit_client.subscribe_cnt == before_subscribe_call + 1);
    int before_callback_call = controller.call_save_message_status;

    rabit_client.callLastCallback(nlohmann::json(test_message_status).dump());
    REQUIRE(controller.call_save_message_status == before_callback_call + 1);
    REQUIRE(controller.last_payload == nlohmann::json(test_message_status).dump());
  }
}
