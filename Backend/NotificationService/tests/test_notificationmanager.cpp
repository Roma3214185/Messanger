#include <catch2/catch_all.hpp>
#include "managers/notificationmanager.h"
#include "mocks/MockRabitMQClient.h"
#include "managers/SocketManager.h"
#include "interfaces/IChatNetworkManager.h"
#include "interfaces/IMessageNetworkManager.h"
#include "interfaces/IUserNetworkManager.h"
#include "NetworkFacade.h"

class MockNetworkManager
    : public IChatNetworkManager
    , public IUserNetworkManager
    , public IMessageNetworkManager {
  public:

};

class MockSocket : public ISocket {
  public:
    void send_text(const std::string& text) override {
      ++send_text_calls;
      last_sended_text = text;
    }

    int send_text_calls = 0;
    std::string last_sended_text = "";
};

TEST_CASE("Test handling messages") {
  MockRabitMQClient mock_rabit_client;
  SocketsManager socket_manager;
  MockNetworkManager network_manager;
  NetworkFacade network_facade = NetworkFactory::create(&network_manager);
  NotificationManager notification_manager(&mock_rabit_client, socket_manager, network_facade);
  MockSocket socket;

  SECTION("Send message to user expected socket receive message") {
    int user_id = 4;
    socket_manager.saveConnections(user_id, &socket);
    Message message_to_send;
    message_to_send.text = "Hi from Socket";

    notification_manager.sendMessageToUser(user_id, message_to_send);

    REQUIRE(socket.send_text_calls == 1);
    REQUIRE(socket.last_sended_text == nlohmann::json(message_to_send).dump());

    SECTION("Delete connection expected socket don't receive sended text") {
      socket_manager.deleteConnections(&socket);
      int before = socket.send_text_calls;

      notification_manager.sendMessageToUser(user_id, message_to_send);

      REQUIRE(socket.send_text_calls == before);
    }
  }
}

class TestNotificationManager : public NotificationManager {
  public:
    using NotificationManager::NotificationManager;
    using NotificationManager::subscribeMessageSaved;

    int handlerCalled = 0;
    std::string lastPayload;

    void handleMessageSaved(const std::string &payload) override {
      handlerCalled++;
      lastPayload = payload;
    }
};

class MockConfigProvider : public IConfigProvider {
  public:
    const Routes& routes() const override {
      return mock_routes;
    }

    const Ports& ports() const override {
      Ports ports;
      return ports;
    }

    const StatusCodes& statusCodes() const override {
      StatusCodes codes;
      return codes;
    }

    Routes mock_routes;
};

TEST_CASE("Test NotificationManager communitacion with RabitMQ") {
  MockRabitMQClient mock_rabit_client;
  SocketsManager socket_manager;
  MockNetworkManager network_manager;

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

  NetworkFacade network_facade = NetworkFactory::create(&network_manager);
  TestNotificationManager notification_manager(&mock_rabit_client, socket_manager, network_facade, &provider);
  MockSocket socket;

  Message message;
  message.id = 1;
  message.text = "hi";

  SECTION("Subscribing to rabitMQ expected rabit handles input data") {
    notification_manager.subscribeMessageSaved();

    REQUIRE(mock_rabit_client.last_subscribe_request.exchange == "test_app.events");
    REQUIRE(mock_rabit_client.last_subscribe_request.exchangeType == "topic");
    REQUIRE(mock_rabit_client.last_subscribe_request.queue == "test_notification_service_queue");
    REQUIRE(mock_rabit_client.last_subscribe_request.routingKey == "test_message_saved");
  }

  SECTION("Subscribing to rabitMQ expected call expected callback") {
    notification_manager.subscribeMessageSaved();
    int before_calls = notification_manager.handlerCalled;

    mock_rabit_client.callLastCallback(nlohmann::json(message).dump());

    REQUIRE(notification_manager.handlerCalled == before_calls + 1);
    REQUIRE(notification_manager.lastPayload == nlohmann::json(message).dump());
  }

  SECTION("On send message expected publish data in rabitMQ") {
    notification_manager.onSendMessage(message);
    auto expected_json = nlohmann::json(message);
    expected_json["event"] = "save_message";

    REQUIRE(mock_rabit_client.last_publish_request.exchange == "test_app.events");
    REQUIRE(mock_rabit_client.last_publish_request.exchangeType == "topic");
    REQUIRE(mock_rabit_client.last_publish_request.message == expected_json.dump());
    REQUIRE(mock_rabit_client.last_publish_request.routingKey == "test_save_message");
  }

}
