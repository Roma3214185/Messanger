#include <catch2/catch_all.hpp>

#include "interfaces/ISocket.h"
#include "notificationservice/managers/notificationmanager.h"
#include "mocks/MockSocket.h"
#include "notificationservice/server.h"
#include "mocks/MockRabitMQClient.h"
#include "mocks/MockNetworkManager.h"
#include "NetworkFacade.h"
#include "notificationservice/managers/socketmanager.h"

class TestServer : public Server {
  public:
    using Server::Server;
    using Server::handleSocketOnMessage;
};

TEST_CASE("handleSocketOnMessage init type registers user") {
  MockRabitMQClient mock_rabit_client;
  SocketsManager socket_manager;
  MockNetworkManager network_manager;

  NetworkFacade network_facade = NetworkFactory::create(&network_manager);
  NotificationManager nm(&mock_rabit_client, &socket_manager, network_facade);

  TestServer server(8080, &nm);
  auto socket = std::make_shared<MockSocket>();

  long long now = getCurrentTime();
  Message new_message;
  new_message.chat_id = 1;
  new_message.sender_id = 2;
  new_message.text = "HIII";
  new_message.timestamp = now;


  SECTION("Handle socket on message receive init request expected user is online") {
    nlohmann::json init_msg;
    init_msg["type"] =  "init";
    init_msg["user_id"]  = 42;

    server.handleSocketOnMessage(socket, init_msg.dump());

    REQUIRE(socket_manager.userOnline(42));
  }

  SECTION("Handle socket on message receive new message expected publishing it in rabiqMQ") {
    nlohmann::json init_msg;
    init_msg["type"] =  std::string("send_message");
    init_msg["id"]  = new_message.id;
    init_msg["chat_id"]  = new_message.chat_id;
    init_msg["sender_id"]  = new_message.sender_id;
    init_msg["local_id"]  = new_message.local_id;
    init_msg["timestamp"]  = now;
    init_msg["text"]  = new_message.text;

    server.handleSocketOnMessage(socket, init_msg.dump());

    nlohmann::json expected_json;
    expected_json["event"] =  "save_message";
    expected_json["id"]  = new_message.id;
    expected_json["chat_id"]  = new_message.chat_id;
    expected_json["sender_id"]  = new_message.sender_id;
    expected_json["local_id"]  = new_message.local_id;
    expected_json["timestamp"]  = now;
    expected_json["text"]  = new_message.text;


    REQUIRE(mock_rabit_client.publish_cnt == 1);
    REQUIRE(mock_rabit_client.last_publish_request.message == expected_json.dump());
  }
}
