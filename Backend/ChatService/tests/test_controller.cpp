#include <catch2/catch_all.hpp>
#include "chatservice/chatserver.h"
#include "chatservice/chatcontroller.h"
#include "mocks/MockChatManager.h"
#include "mocks/MockNetworkManager.h"
#include "mocks/MockConfigProvider.h"
#include "NetworkFacade.h"
#include "chatservice/AutoritizerProvider.h"
#include "mocks/MockUtils.h"

#include "ProdConfigProvider.h"

namespace  Controller {

struct TestFixture {
    crow::SimpleApp app;
    MockChatManager manager;
    MockNetworkManager network_manager;
    MockConfigProvider provider;
    NetworkFacade facade = NetworkFactory::create(&network_manager);
    ChatController controller;
    ChatServer server;
    int port = 100;
    std::string secret_token = "Secret-token-123";
    crow::request req;
    crow::response res;
    int user_id = 13;
    std::shared_ptr<MockAutoritizer> mock_autoritized;

    TestFixture()
        : controller(&manager, &facade, &provider)
        , server(app, port, &controller) {
      mock_autoritized = std::make_shared<MockAutoritizer>();
      AutoritizerProvider::set(mock_autoritized);
      mock_autoritized->mock_user_id = user_id;
      provider.mock_codes =  MockUtils::getMockCodes();
      req.add_header("Authorization", secret_token);
    }
};

}  //Controller

TEST_CASE("Test createPrivateChat") {
  Controller::TestFixture fix;

  SECTION("Request without body expected return userError status code and info about issue") {
    fix.controller.createPrivateChat(fix.req, fix.res);
    REQUIRE(fix.res.code == fix.provider.statusCodes().userError);
    REQUIRE(fix.res.body == "Missing user_id value");
  }

  SECTION("Request body don't have user_id fiels expected return userError status code and info about issue") {
    fix.req.body = R"({"users_id": 1})";
    fix.controller.createPrivateChat(fix.req, fix.res);
    REQUIRE(fix.res.code == fix.provider.statusCodes().userError);
    REQUIRE(fix.res.body == "Missing user_id value");
  }

  int other_user_id = 37;
  fix.req.body = R"({"user_id": 37})";

  SECTION("Request body have user_id fiels expected call to network_manager with valid id") {
    fix.controller.createPrivateChat(fix.req, fix.res);
    REQUIRE(fix.network_manager.last_user_id == other_user_id);
  }

  SECTION("Network_manager not found user expected userError and output issue") {
    fix.controller.createPrivateChat(fix.req, fix.res);
    REQUIRE(fix.res.code == fix.provider.statusCodes().userError);
    REQUIRE(fix.res.body == fix.provider.statusCodes().userNotFound);
  }

  SECTION("Network_manager not found user expected userError and output issue") {
    fix.controller.createPrivateChat(fix.req, fix.res);
    REQUIRE(fix.res.code == fix.provider.statusCodes().userError);
    REQUIRE(fix.res.body == fix.provider.statusCodes().userNotFound);
  }

  User user;
  user.username = "Roma";
  user.id = 123;
  user.avatar = "path/to/avatar";
  fix.network_manager.mock_user = user;
  SECTION("Network_manager get valid ids") {
    int before = fix.manager.call_createPrivateChat;

    fix.controller.createPrivateChat(fix.req, fix.res);

    REQUIRE(fix.manager.call_createPrivateChat == before + 1);
    REQUIRE(fix.manager.last_createPrivateChat.first == fix.user_id);
    REQUIRE(fix.manager.last_createPrivateChat.second == other_user_id);
  }

  SECTION("Chat not created expected serverError status code and problem issue") {
    int before = fix.manager.call_createPrivateChat;

    fix.controller.createPrivateChat(fix.req, fix.res);

    REQUIRE(fix.manager.call_createPrivateChat == before + 1);
    REQUIRE(fix.res.code == fix.provider.statusCodes().serverError);
    REQUIRE(fix.res.body == "Failed to create chat");
  }

  int chat_id = 12;
  fix.manager.mock_chat_id = chat_id;

  SECTION("Chat created expected success status code and valid json outpit") {
    int before = fix.manager.call_createPrivateChat;

    fix.controller.createPrivateChat(fix.req, fix.res);

    REQUIRE(fix.manager.call_createPrivateChat == before + 1);
    REQUIRE(fix.res.code == fix.provider.statusCodes().success);

    auto r = crow::json::load(fix.res.body);

    REQUIRE(r.size() == 3);
    CHECK(r["id"].i() == chat_id);
    CHECK(r["type"].s()    == "private");
    CHECK(r["user"]["id"].i()   == user.id);
    CHECK(r["user"]["avatar"].s()  == user.avatar);
    CHECK(r["user"]["name"].s() == user.username);
  }
}

TEST_CASE("Test "){}

/*




*/
