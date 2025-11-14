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

TEST_CASE("First test") {
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
