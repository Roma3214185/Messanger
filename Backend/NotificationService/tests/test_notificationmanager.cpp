// #include <catch2/catch_all.hpp>
// #include "notificationservice/managers/notificationmanager.h"
// #include "mocks/MockRabitMQClient.h"
// #include "notificationservice/managers/socketmanager.h"
// #include "interfaces/IChatNetworkManager.h"
// #include "interfaces/IMessageNetworkManager.h"
// #include "interfaces/IUserNetworkManager.h"
// #include "NetworkFacade.h"
// #include "mocks/notificationservice/MockSocket.h"
// #include "mocks/MockConfigProvider.h"
// #include "mocks/MockNetworkManager.h"

// #include <unordered_map>

// TEST_CASE("Test handling messages") {
//   MockRabitMQClient mock_rabit_client;
//   SocketsManager socket_manager;
//   MockNetworkManager network_manager;
//   NetworkFacade network_facade = NetworkFactory::create(&network_manager);
//   NotificationManager notification_manager(&mock_rabit_client, &socket_manager, network_facade);
//   int user_id = 4;

//   SECTION("Send message to user expected socket receive message") {
//     auto socket = std::make_shared<MockSocket>();
//     socket_manager.saveConnections(user_id, socket);
//     Message message_to_send;
//     message_to_send.text = "Hi from Socket";
//     auto expected_json = nlohmann::json(message_to_send);
//     expected_json["type"] = "new_message";

//     bool result = notification_manager.notifyMember(user_id, message_to_send, "new_message");

//     REQUIRE(result);
//     REQUIRE(socket->send_text_calls == 1);
//     REQUIRE(socket->last_sended_text == expected_json.dump());
//   }

//   SECTION("User offline expected socket don't receive sended text") {
//     Message message_to_send;
//     bool result = notification_manager.notifyMember(user_id, message_to_send, "new_message");

//     REQUIRE_FALSE(result);
//   }
// }

// class TestNotificationManager : public NotificationManager {
//   public:
//     using NotificationManager::NotificationManager;
//     using NotificationManager::subscribeMessageSaved;
// };

// TEST_CASE("Test NotificationManager communitacion with RabitMQ") {
//   MockRabitMQClient mock_rabit_client;
//   SocketsManager socket_manager;
//   MockNetworkManager network_manager;

//   Routes mock_routes;
//   // mock_routes.messageSaved            = "test_message_saved";
//   // mock_routes.saveMessage             = "test_save_message";
//   // mock_routes.saveMessageStatus       = "test_save_message_status";
//   // mock_routes.messageSavedQueue       = "test_message_saved_queue";
//   // mock_routes.messageStatusSavedQueue = "test_message_status_saved_queue";
//   // mock_routes.saveMessageQueue        = "test_save_message_queue";
//   // mock_routes.saveMessageStatusQueue  = "test_save_message_status_queue";
//   // mock_routes.exchange                = "test_app.events";
//   // mock_routes.messageStatusSaved      = "test_message_status_saved";
//   // mock_routes.exchangeType            = "test_topic";

//   MockConfigProvider provider;
//   provider.mock_routes = mock_routes;

//   NetworkFacade network_facade = NetworkFactory::create(&network_manager);
//   TestNotificationManager notification_manager(&mock_rabit_client, &socket_manager,
//   network_facade, &provider); MockSocket socket;

//   Message message;
//   message.id = 1;
//   message.text = "hi";

//   SECTION("Subscribing to rabitMQ expected rabit handles input data") {
//     notification_manager.subscribeMessageSaved();

//     CHECK(mock_rabit_client.last_subscribe_request.exchange == mock_routes.exchange);
//     CHECK(mock_rabit_client.last_subscribe_request.exchange_type == mock_routes.exchangeType);
//     CHECK(mock_rabit_client.last_subscribe_request.queue == mock_routes.messageSavedQueue);
//     CHECK(mock_rabit_client.last_subscribe_request.routing_key == mock_routes.messageSaved);
//   }

//   SECTION("Subscribing to rabitMQ expected call expected callback") {
//     notification_manager.subscribeMessageSaved();
//     int before_calls = notification_manager.handlerCalled;

//     mock_rabit_client.callLastCallback(nlohmann::json(message).dump());

//     REQUIRE(notification_manager.handlerCalled == before_calls + 1);
//     REQUIRE(notification_manager.lastPayload == nlohmann::json(message).dump());
//   }

//   SECTION("On send message expected publish data in rabitMQ") {
//     notification_manager.onSendMessage(message);
//     auto expected_json = nlohmann::json(message);
//     expected_json["event"] = "save_message";

//     CHECK(mock_rabit_client.last_publish_request.exchange == mock_routes.exchange);
//     CHECK(mock_rabit_client.last_publish_request.exchange_type == mock_routes.exchangeType);
//     CHECK(mock_rabit_client.last_publish_request.message == expected_json.dump());
//     CHECK(mock_rabit_client.last_publish_request.routing_key == mock_routes.saveMessage);
//   }

// }

// class TestNotificationManager1 : public NotificationManager{
//   public:
//     using NotificationManager::NotificationManager;
//     using NotificationManager::handleMessageSaved;
// };

// TEST_CASE("Notification manager handles message saved") {
//   MockRabitMQClient mock_rabit_client;
//   SocketsManager socket_manager;
//   MockNetworkManager network_manager;

//   NetworkFacade network_facade = NetworkFactory::create(&network_manager);
//   TestNotificationManager1 notification_manager(&mock_rabit_client, &socket_manager,
//   network_facade);

//   auto socket1 = std::make_shared<MockSocket>();
//   auto socket2 = std::make_shared<MockSocket>();
//   auto socket3 = std::make_shared<MockSocket>();
//   auto socket4 = std::make_shared<MockSocket>();
//   auto socket0 = std::make_shared<MockSocket>();

//   std::vector<long long> user_ids(5);
//   user_ids[0] = 1;
//   user_ids[1] = 2;
//   user_ids[2] = 3;
//   user_ids[3] = 4;
//   user_ids[4] = 5;

//   socket_manager.saveConnections(user_ids[0], socket0);
//   socket_manager.saveConnections(user_ids[1], socket1);
//   socket_manager.saveConnections(user_ids[2], socket2);
//   socket_manager.saveConnections(user_ids[3], socket3);
//   socket_manager.saveConnections(user_ids[4], socket4);

//   long long chat_id = 5;
//   network_manager.setChatMembers(chat_id, user_ids);

//   Message message;
//   message.id = 1;
//   message.chat_id = chat_id;
//   message.text = "hi";

//   SECTION("Handle message all online expected socket receive to sending all messages") {
//     Message message_to_save;
//     message_to_save.id = 1;
//     message_to_save.chat_id = chat_id;
//     message_to_save.text = "hi2";
//     notification_manager.handleMessageSaved(nlohmann::json(message_to_save).dump());

//     REQUIRE(socket0->send_text_calls == 1);
//     REQUIRE(socket1->send_text_calls == 1);
//     REQUIRE(socket2->send_text_calls == 1);
//     REQUIRE(socket3->send_text_calls == 1);
//     REQUIRE(socket4->send_text_calls == 1);
//   }

//   SECTION("Handle message all online expect one expected socket receive to sending all messages -
//   1") {
//     socket_manager.deleteConnections(socket0);

//     notification_manager.handleMessageSaved(nlohmann::json(message).dump());

//     REQUIRE(socket0->send_text_calls == 0);
//     REQUIRE(socket1->send_text_calls == 1);
//     REQUIRE(socket2->send_text_calls == 1);
//     REQUIRE(socket3->send_text_calls == 1);
//     REQUIRE(socket4->send_text_calls == 1);
//   }

//   SECTION("Handle message receive invalid payload expected not sending any message") {
//     int before_cnt = mock_rabit_client.publish_cnt;

//     notification_manager.handleMessageSaved("invalid payload");

//     REQUIRE(mock_rabit_client.publish_cnt == before_cnt);
//   }

//   SECTION("Handle message all online expected all publishing to save message_status in RabiqMQ")
//   {
//     int before = mock_rabit_client.getPublishCnt("save_message_status");
//     int before_cnt = mock_rabit_client.publish_cnt;

//     notification_manager.handleMessageSaved(nlohmann::json(message).dump());

//     REQUIRE(mock_rabit_client.publish_cnt == before_cnt + user_ids.size());
//   }

//   SECTION("Handle message all online except one expected all publishing to save message_status in
//   RabiqMQ") {
//     socket_manager.deleteConnections(socket0);
//     int before = mock_rabit_client.getPublishCnt("save_message_status");
//     int before_cnt = mock_rabit_client.publish_cnt;

//     notification_manager.handleMessageSaved(nlohmann::json(message).dump());

//     REQUIRE(mock_rabit_client.publish_cnt == before_cnt + user_ids.size());
//   }
// }
