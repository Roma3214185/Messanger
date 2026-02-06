#include <catch2/catch_all.hpp>
#include "mocks/MockRabitMQClient.h"
#include "interfaces/IChatNetworkManager.h"
#include "interfaces/IMessageNetworkManager.h"
#include "interfaces/IUserNetworkManager.h"
#include "NetworkFacade.h"
#include "mocks/notificationservice/MockSocket.h"
#include "mocks/MockNetworkManager.h"
#include "notificationservice/SocketRepository.h"
#include "notificationservice/managers/NotificationOrchestrator.h"
#include "notificationservice/SocketNotifier.h"
#include "notificationservice/IPublisher.h"
#include "config/Routes.h"
#include <unordered_map>

class MockNotifier : public INotifier {
public:
    bool notifyMember(long long user_id, nlohmann::json json_message, std::string type) override {

    }
};

class MockPublisher : public IPublisher {
    public:
    void saveMessageStatus(MessageStatus &status) override {

    }

    void saveDeliveryStatus(const Message &msg, long long receiver_id) override {

    }

    void deleteReaction(const Reaction &reaction) override {

    }

    void saveMessage(const Message &message) override {

    }

    void saveReaction(const Reaction &reaction) override {

    }
};

class MockRepository : public IActiveSocketRepository, public IUserSocketRepository {
public:
    SocketPtr findSocket(crow::websocket::connection *conn) override {

    }

    void addConnection(const SocketPtr &socket) override {

    }
    void deleteConnection(const SocketPtr &socket) override {

    }
    void saveConnections(UserId, SocketPtr socket) override {

    }
    SocketPtr getUserSocket(UserId) override {

    }
    bool userOnline(UserId) override {

    }
};

class MockPublicher  : public IPublisher{
public:
    void saveMessageStatus(MessageStatus &status) override {

    }
    void saveDeliveryStatus(const Message &msg, long long receiver_id) override {

    }
    void saveReaction(const Reaction &reaction) override {

    }
    void deleteReaction(const Reaction &reaction) override {

    }
    void saveMessage(const Message &message) override {

    }
};


struct NotificationTestsFixture {
    MockRabitMQClient mock_rabit_client;
    SocketRepository socket_manager;
    MockNetworkManager network_manager;
    MockNotifier notifier;
    MockFacade facade;
    NotificationOrchestrator notification_manager;
    int user_id = 4;
    MockPublisher publisher;

    NotificationTestsFixture() :
        facade(network_manager),
        notification_manager(&facade,
                             &publisher, &notifier) {

    }
};

TEST_CASE("Test NotificationManager communitacion with RabitMQ") {
  NotificationTestsFixture fix;
  RabbitNotificationPublisher publisher(&fix.mock_rabit_client);
  Message message;
  message.id = 1;
  message.text = "hi";
  publisher.saveMessage(message);

  SECTION("Save message expected create right publish request") {
      nlohmann::json expected = nlohmann::json(message);
      expected["event"] = "save_message";

    CHECK(fix.mock_rabit_client.last_publish_request.exchange ==
    Config::Routes::exchange);
    CHECK(fix.mock_rabit_client.last_publish_request.exchange_type ==
    Config::Routes::exchangeType);
    CHECK(fix.mock_rabit_client.last_publish_request.message ==
    expected.dump());
    CHECK(fix.mock_rabit_client.last_publish_request.routing_key ==
    Config::Routes::saveMessage);
  }
}
