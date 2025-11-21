#include <catch2/catch_all.hpp>
#include "chatservice/chatserver.h"
#include "chatservice/chatcontroller.h"
#include "mocks/MockChatManager.h"
#include "mocks/MockNetworkManager.h"
#include "mocks/MockConfigProvider.h"
#include "NetworkFacade.h"
#include "chatservice/AutoritizerProvider.h"
#include "mocks/MockUtils.h"
#include "mocks/MockAutoritizer.h"

#include "ProdConfigProvider.h"

namespace TestController {

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
      auth();
    }

    void auth() {
      mock_autoritized->mock_user_id = user_id;
      provider.mock_codes =  MockUtils::getMockCodes();
      req.add_header("Authorization", secret_token);

      provider.mock_issue_message.invalidToken = "test_invalid_messages";
      provider.mock_issue_message.userNotFound = "test_user_not_found";
    }
};

} // namespace TestController

TEST_CASE("Test createPrivateChat") {
  TestController::TestFixture fix;

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
    REQUIRE(fix.res.body == fix.provider.issueMessages().userNotFound);
  }

  SECTION("Network_manager not found user expected userError and output issue") {
    fix.controller.createPrivateChat(fix.req, fix.res);
    REQUIRE(fix.res.code == fix.provider.statusCodes().userError);
    REQUIRE(fix.res.body == fix.provider.issueMessages().userNotFound);
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

TEST_CASE("Test getAllChatMembers") {
  TestController::TestFixture fix;
  int chat_id = 10;

  SECTION("Invalid token expected no error about invalid token") {
    fix.mock_autoritized->need_fail = true;
    fix.controller.getAllChatMembers(fix.req, fix.res, chat_id);
    REQUIRE(fix.res.body != fix.provider.issueMessages().invalidToken);
  }

  SECTION("DB returns empty list expected serverError") {
    fix.manager.mock_members.clear();

    fix.controller.getAllChatMembers(fix.req, fix.res, chat_id);

    REQUIRE(fix.res.code == fix.provider.statusCodes().serverError);
    REQUIRE(fix.res.body == "Error in db.getMembersOfChat");
  }

  SECTION("DB returns one member expected success and correct json") {
    fix.manager.mock_members = { 5 };

    fix.controller.getAllChatMembers(fix.req, fix.res, chat_id);

    REQUIRE(fix.res.code == fix.provider.statusCodes().success);

    auto r = crow::json::load(fix.res.body);

    REQUIRE(r.has("members"));
    REQUIRE(r["members"].size() == 1);
    CHECK(r["members"][0].i() == 5);
  }

  SECTION("DB returns multiple members expected success and correct json") {
    std::vector<long long> members = { 3, 7, 12 };
    fix.manager.mock_members = members;

    fix.controller.getAllChatMembers(fix.req, fix.res, chat_id);

    REQUIRE(fix.res.code == fix.provider.statusCodes().success);

    auto r = crow::json::load(fix.res.body);

    REQUIRE(r.has("members"));
    REQUIRE(r["members"].size() == members.size());

    for (int i = 0; i < members.size(); i++) {
      CHECK(r["members"][i].i() == members[i]);
    }
  }
}

TEST_CASE("Test ChatController::GetChat") {
  TestController::TestFixture fix;
  int chat_id = 12;

  SECTION("Invalid token expected info about it") {
    fix.mock_autoritized->need_fail = true;
    fix.controller.getChat(fix.req, fix.res, chat_id);
    REQUIRE(fix.res.code == fix.provider.statusCodes().userError);
    REQUIRE(fix.res.body == fix.provider.issueMessages().invalidToken);
  }

  SECTION("Chat not found expected statusCode userError and info about issue") {
    fix.controller.getChat(fix.req, fix.res, chat_id);
    REQUIRE(fix.res.code == fix.provider.statusCodes().userError);
    REQUIRE(fix.res.body == "Chat not found");
  }

  auto now = QDateTime::currentSecsSinceEpoch();
  Chat private_chat;
  private_chat.id = chat_id;
  private_chat.name = "This will not use here";
  private_chat.is_group = false;
  private_chat.created_at = now;
  fix.manager.mock_chat = private_chat;
  fix.manager.mock_chat_by_id = {{private_chat.id, private_chat}};

  SECTION("Not found other user for private chat") {
    fix.controller.getChat(fix.req, fix.res, chat_id);
    REQUIRE(fix.res.code == fix.provider.statusCodes().notFound);
    REQUIRE(fix.res.body == "Other user not found for this chat");
  }

  int other_member_id = 123;
  fix.manager.mock_other_member_id = other_member_id;

  SECTION("Not found user profile expected badRequest error and problem issue") {
    fix.controller.getChat(fix.req, fix.res, chat_id);
    REQUIRE(fix.res.code == fix.provider.statusCodes().badRequest);
    REQUIRE(fix.res.body == "User profile not found");
  }

  User other_user;
  other_user.id = 1245;
  other_user.username = "Otheruser username";
  other_user.avatar = "other/avatar/path";
  fix.network_manager.mock_user = other_user;

  SECTION("Expected success status code and valid json responce") {
    fix.controller.getChat(fix.req, fix.res, chat_id);
    REQUIRE(fix.res.code == fix.provider.statusCodes().success);


    auto r = crow::json::load(fix.res.body);

    REQUIRE(r.size() == 3);
    CHECK(r["id"].i() == chat_id);
    CHECK(r["type"].s()    == "private");
    CHECK(r["user"]["id"].i()   == other_user.id);
    CHECK(r["user"]["avatar"].s()  == other_user.avatar);
    CHECK(r["user"]["name"].s() == other_user.username);
  }
}

TEST_CASE("Test ChatController::GetAllChats") {
  TestController::TestFixture fix;

  SECTION("Invalid token → userError and invalidToken message") {
    fix.mock_autoritized->need_fail = true;

    fix.controller.getAllChats(fix.req, fix.res);

    REQUIRE(fix.res.code == fix.provider.statusCodes().userError);
    REQUIRE(fix.res.body == fix.provider.issueMessages().invalidToken);
  }

  fix.manager.mock_chat_ids = {1, 2};

  Chat private_chat;
  private_chat.id = 1;
  private_chat.is_group = false;
  private_chat.created_at = QDateTime::currentSecsSinceEpoch();

  Chat group_chat;
  group_chat.id = 2;
  group_chat.is_group = true;
  group_chat.name = "GroupChatName";
  group_chat.avatar = "group/avatar.png";
  group_chat.created_at = QDateTime::currentSecsSinceEpoch();

  fix.manager.mock_chat_by_id = {
      {private_chat.id, private_chat},
      {group_chat.id, group_chat}
  };

  SECTION("Chat not found (getChatById returns 0) expected failure") {
    fix.manager.mock_chat_by_id.clear();

    fix.controller.getAllChats(fix.req, fix.res);

    CHECK(fix.res.code == fix.provider.statusCodes().userError);
    CHECK(fix.res.body == "Chat not found");
  }

  SECTION("Private chat: missing other user expected immediate failure") {
    fix.manager.mock_chat_by_id = {{private_chat.id, private_chat}};
    fix.manager.mock_other_member_id = std::nullopt;

    fix.controller.getAllChats(fix.req, fix.res);

    CHECK(fix.res.code == fix.provider.statusCodes().notFound);
    CHECK(fix.res.body == "Other user not found for this chat");
  }

  SECTION("Private chat: other user found but user profile missing expected immediate failure") {
    fix.manager.mock_chat = private_chat;
    fix.manager.mock_other_member_id = 123;
    fix.network_manager.mock_user = std::nullopt;

    fix.controller.getAllChats(fix.req, fix.res);

    CHECK(fix.res.code == fix.provider.statusCodes().badRequest);
    CHECK(fix.res.body == "User profile not found");
  }

  User other_user;
  other_user.id = 123;
  other_user.username = "OtherUserName";
  other_user.avatar = "avatar/path";
  fix.network_manager.mock_user = other_user;
  fix.manager.mock_other_member_id = 123;

  SECTION("Group chat: member count retrieval fails → immediate failure") {
    fix.manager.mock_chat = group_chat;
    fix.manager.mock_cht = 0;

    fix.controller.getAllChats(fix.req, fix.res);

    CHECK(fix.res.code == fix.provider.statusCodes().serverError);
    CHECK(fix.res.body == "Failed to retrieve group member count");
  }

  SECTION("One valid private chat and one valid group chat expected success") {
    fix.auth();
    fix.manager.mock_chat_by_id = {
        {private_chat.id, private_chat},
        {group_chat.id, group_chat}
    };
    fix.manager.mock_other_member_id = other_user.id;
    fix.network_manager.mock_user = other_user;
    fix.manager.mock_cht = 5;

    fix.controller.getAllChats(fix.req, fix.res);

    REQUIRE(fix.res.code == fix.provider.statusCodes().success);

    auto r = crow::json::load(fix.res.body);
    REQUIRE(r["chats"].size() == 2);

    auto c0 = r["chats"][0];
    CHECK(c0["id"].i() == private_chat.id);
    CHECK(c0["type"].s() == "private");
    CHECK(c0["user"]["id"].i() == other_user.id);
    CHECK(c0["user"]["name"].s() == other_user.username);
    CHECK(c0["user"]["avatar"].s() == other_user.avatar);

    auto c1 = r["chats"][1];
    CHECK(c1["id"].i() == group_chat.id);
    CHECK(c1["type"].s() == "group");
    CHECK(c1["name"].s() == group_chat.name);
    CHECK(c1["avatar"].s() == group_chat.avatar);
    CHECK(c1["member_count"].i() == 5);
  }
}
