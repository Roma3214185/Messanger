#include <catch2/catch_all.hpp>
#include "mocks/MockRabitMQClient.h"
#include "notificationservice/IPublisher.h"
#include "config/Routes.h"
#include "entities/Message.h"
#include "entities/MessageStatus.h"
#include "entities/Reaction.h"

TEST_CASE("Test NotificationManager communitacion with RabitMQ") {
  MockRabitMQClient mock_rabit_client;
  RabbitNotificationPublisher publisher(&mock_rabit_client);

  auto expectedJson = [&](auto element) {
      return nlohmann::json(element).dump();
  };

  SECTION("Save message expected create right publish request") {
    Message message;
    message.id = 1;
    message.text = "hi";

    publisher.saveMessage(message);

    CHECK(mock_rabit_client.last_publish_request.exchange ==
    Config::Routes::exchange);
    CHECK(mock_rabit_client.last_publish_request.exchange_type ==
    Config::Routes::exchangeType);
    CHECK(mock_rabit_client.last_publish_request.message ==
          expectedJson(message));
    CHECK(mock_rabit_client.last_publish_request.routing_key ==
    Config::Routes::saveMessage);
  }

  SECTION("Save message_status expected create right publish request") {
      MessageStatus message_status;
      message_status.receiver_id = 21;

      publisher.saveMessageStatus(message_status);

      CHECK(mock_rabit_client.last_publish_request.exchange ==
            Config::Routes::exchange);
      CHECK(mock_rabit_client.last_publish_request.exchange_type ==
            Config::Routes::exchangeType);
      CHECK(mock_rabit_client.last_publish_request.message ==
            expectedJson(message_status));
      CHECK(mock_rabit_client.last_publish_request.routing_key ==
            Config::Routes::saveMessageStatus);
  }

  SECTION("Save reaction expected create right publish request") {
      Reaction reaction;
      reaction.message_id = 12;
      reaction.reaction_id = 1231;

      publisher.saveReaction(reaction);

      CHECK(mock_rabit_client.last_publish_request.exchange ==
            Config::Routes::exchange);
      CHECK(mock_rabit_client.last_publish_request.exchange_type ==
            Config::Routes::exchangeType);
      CHECK(mock_rabit_client.last_publish_request.message ==
            expectedJson(reaction));
      CHECK(mock_rabit_client.last_publish_request.routing_key ==
            Config::Routes::saveReaction);
  }

  SECTION("Delete reaction expected create right publish request") {
      Reaction reaction;
      reaction.message_id = 12;
      reaction.reaction_id = 1231;

      publisher.deleteReaction(reaction);

      CHECK(mock_rabit_client.last_publish_request.exchange ==
            Config::Routes::exchange);
      CHECK(mock_rabit_client.last_publish_request.exchange_type ==
            Config::Routes::exchangeType);
      CHECK(mock_rabit_client.last_publish_request.message ==
            expectedJson(reaction));
      CHECK(mock_rabit_client.last_publish_request.routing_key ==
            Config::Routes::deleteReaction);
    }
}

// #include <catch2/catch_all.hpp>
// #include "mocks/MockRabitMQClient.h"
// #include "interfaces/IMessageNetworkManager.h"
// #include "interfaces/IUserNetworkManager.h"
// #include "NetworkFacade.h"
// #include "mocks/notificationservice/MockSocket.h"
// #include "mocks/MockNetworkManager.h"
// #include "notificationservice/SocketRepository.h"
// #include "notificationservice/managers/NotificationOrchestrator.h"
// #include "notificationservice/SocketNotifier.h"
// #include "notificationservice/IPublisher.h"
// #include "config/Routes.h"
// #include <unordered_map>
// class MockNotifier : public INotifier {
// public:
//     bool notifyMember(long long user_id, nlohmann::json json_message, std::string type) override {

//     }
// };

// class MockRepository : public IActiveSocketRepository, public IUserSocketRepository {
// public:
//     SocketPtr findSocket(crow::websocket::connection *conn) override {

//     }

//     void addConnection(const SocketPtr &socket) override {

//     }
//     void deleteConnection(const SocketPtr &socket) override {

//     }
//     void saveConnections(UserId, SocketPtr socket) override {

//     }
//     SocketPtr getUserSocket(UserId) override {

//     }
//     bool userOnline(UserId) override {

//     }
// };

// class MockPublisher  : public IPublisher{
// public:
//     void saveMessageStatus(MessageStatus &status) override {

//     }

//     void saveReaction(const Reaction &reaction) override {

//     }

//     void deleteReaction(const Reaction &reaction) override {

//     }

//     void saveMessage(const Message &message) override {


//     }
// };


// struct NotificationTestsFixture {
//     MockRabitMQClient mock_rabit_client;
//     SocketRepository socket_manager;
//     MockNetworkManager network_manager;
//     MockNotifier notifier;
//     MockFacade facade;
//     NotificationOrchestrator notification_manager;
//     int user_id = 4;
//     MockPublisher publisher;

//     NotificationTestsFixture() :
//         facade(network_manager),
//         notification_manager(&facade,
//                              &publisher, &notifier) {

//     }

// };
