#include <catch2/catch_all.hpp>
#include "managers/notificationmanager.h"
#include "mocks/MockRabitMQClient.h"
#include "managers/SocketManager.h"
#include "interfaces/IChatNetworkManager.h"
#include "interfaces/IMessageNetworkManager.h"
#include "interfaces/IUserNetworkManager.h"
#include "NetworkFacade.h"
#include "mocks/MockSocket.h"
#include "mocks/MockConfigProvider.h"

#include <unordered_map>
#include <QVector>

class MockNetworkManager
    : public IChatNetworkManager
    , public IUserNetworkManager
    , public IMessageNetworkManager {
    std::unordered_map<int, QVector<int>> mp;
  public:
    void setChatMembers(int chat_id, QVector<int> ids) {
      mp[chat_id] = ids;
    }

    QVector<UserId> getMembersOfChat(int chat_id) override {
      return mp[chat_id];
    }
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
    auto expected_json = nlohmann::json(message_to_send);
    expected_json["type"] = "new_message";

    bool result = notification_manager.notifyMember(user_id, message_to_send);

    REQUIRE(result);
    REQUIRE(socket.send_text_calls == 1);
    REQUIRE(socket.last_sended_text == expected_json.dump());

    SECTION("Delete connection expected socket don't receive sended text") {
      socket_manager.deleteConnections(&socket);
      int before = socket.send_text_calls;

      bool result = notification_manager.notifyMember(user_id, message_to_send);

      REQUIRE_FALSE(result);
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

class TestNotificationManager1 : public NotificationManager{
  public:
    using NotificationManager::NotificationManager;
    using NotificationManager::handleMessageSaved;
};

TEST_CASE("Notification manager handles message saved") {
  MockRabitMQClient mock_rabit_client;
  SocketsManager socket_manager;
  MockNetworkManager network_manager;

  NetworkFacade network_facade = NetworkFactory::create(&network_manager);
  TestNotificationManager1 notification_manager(&mock_rabit_client, socket_manager, network_facade);

  QVector<MockSocket> sockets;
  MockSocket s1, s2, s3, s4, s5;
  sockets.emplace_back(s1);
  sockets.emplace_back(s2);
  sockets.emplace_back(s3);
  sockets.emplace_back(s4);
  sockets.emplace_back(s4);

  QVector<int> user_ids;
  user_ids.emplace_back(1);
  user_ids.emplace_back(2);
  user_ids.emplace_back(3);
  user_ids.emplace_back(4);
  user_ids.emplace_back(5);

  REQUIRE(user_ids.size() == sockets.size());

  for(int i = 0; i < user_ids.size(); i++) {
    socket_manager.saveConnections(user_ids[i], &sockets[i]);
  }

  int chat_id = 5;
  network_manager.setChatMembers(5, user_ids);

  Message message;
  message.id = 1;
  message.chat_id = chat_id;
  message.text = "hi";

  SECTION("Handle message all online expected socket receive to sending all messages") {
    notification_manager.handleMessageSaved(nlohmann::json(message).dump());

    for(int i = 0; i < sockets.size(); i++) {
      REQUIRE(sockets[i].send_text_calls == 1);
    }
  }

  SECTION("Handle message all online expect one expected socket receive to sending all messages - 1") {
    socket_manager.deleteConnections(&sockets[0]);

    notification_manager.handleMessageSaved(nlohmann::json(message).dump());

    int cnt = 0;
    for(auto& socket: sockets) {
      cnt += socket.send_text_calls;
    }

    REQUIRE(cnt == sockets.size() - 1);
  }

  SECTION("Handle message receive invalid payload expected not sending any message") {
    int before_cnt = mock_rabit_client.publish_cnt;

    notification_manager.handleMessageSaved("invalid payload");

    REQUIRE(mock_rabit_client.publish_cnt == before_cnt);
  }

  SECTION("Handle message all online expected all publishing to save message_status in RabiqMQ") {
    int before = mock_rabit_client.getPublishCnt("save_message_status");
    int before_cnt = mock_rabit_client.publish_cnt;

    notification_manager.handleMessageSaved(nlohmann::json(message).dump());

    REQUIRE(mock_rabit_client.publish_cnt == before_cnt + sockets.size());
  }

  SECTION("Handle message all online except one expected all publishing to save message_status in RabiqMQ") {
    socket_manager.deleteConnections(&sockets[0]);
    int before = mock_rabit_client.getPublishCnt("save_message_status");
    int before_cnt = mock_rabit_client.publish_cnt;

    notification_manager.handleMessageSaved(nlohmann::json(message).dump());

    REQUIRE(mock_rabit_client.publish_cnt == before_cnt + sockets.size());
  }
}
