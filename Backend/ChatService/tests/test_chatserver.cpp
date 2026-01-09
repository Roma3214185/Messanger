#include <catch2/catch_all.hpp>

#include "NetworkFacade.h"
#include "chatservice/AutoritizerProvider.h"
#include "chatservice/chatcontroller.h"
#include "chatservice/chatserver.h"
#include "mocks/MockAutoritizer.h"
#include "mocks/MockNetworkManager.h"
#include "mocks/chatservice/MockChatManager.h"

namespace TestServer {

struct TestFixture {
  crow::SimpleApp app;
  MockChatManager manager;
  MockNetworkManager network_manager;
  NetworkFacade facade = NetworkFactory::create(&network_manager);
  ChatController controller;
  ChatServer server;
  int port = 100;
  std::string secret_token = "Secret-token-123";
  crow::request req;
  crow::response res;
  int user_id = 13;
  std::shared_ptr<MockAutoritizer> mock_autoritized;

  TestFixture() : controller(&manager, &facade), server(app, 100, &controller) {
    mock_autoritized = std::make_shared<MockAutoritizer>();
    AutoritizerProvider::set(mock_autoritized);
    mock_autoritized->mock_user_id = user_id;
  }

  std::string formError(const std::string &text) {
    nlohmann::json json;
    json["error"] = text;
    return json.dump();
  }
};

}  // namespace TestServer

TEST_CASE("handleCreatingPrivateChat listens on POST /chats/private") {
  TestServer::TestFixture fix;
  SECTION("Invalid token expected not call getChatsOfUser") {
    fix.mock_autoritized->need_fail = true;
    fix.app.validate();
    fix.req.method = "GET"_method;
    fix.req.url = "/chats";
    int before = fix.manager.call_getChatsOfUser;

    fix.app.handle_full(fix.req, fix.res);

    REQUIRE(fix.manager.call_getChatsOfUser == before);
    REQUIRE(fix.res.code == Config::StatusCodes::userError);
    REQUIRE(fix.res.body == fix.formError(Config::IssueMessages::invalidToken));
  }

  fix.req.add_header("Authorization", fix.secret_token);

  SECTION("Token is setted expected call getChatsOfUser") {
    fix.app.validate();
    fix.req.method = "GET"_method;
    fix.req.url = "/chats";
    int before_getChatCall = fix.manager.call_getChatsOfUser;
    int before_auth_call = fix.mock_autoritized->call_autoritize;

    fix.app.handle_full(fix.req, fix.res);

    REQUIRE(fix.mock_autoritized->call_autoritize == before_auth_call + 1);
    REQUIRE(fix.mock_autoritized->last_token == fix.secret_token);
    REQUIRE(fix.manager.call_getChatsOfUser == before_getChatCall + 1);
    REQUIRE(fix.manager.last_user_id == fix.user_id);
  }
}

TEST_CASE(
    "handleGetChat listens on GET /chats/<int> and call Manager::GetChat "
    "with expected chat_id") {
  TestServer::TestFixture fix;
  fix.req.add_header("Authorization", fix.secret_token);
  fix.app.validate();
  fix.req.method = "GET"_method;
  fix.req.url = "/chats/89";
  int before = fix.manager.call_getChatById;
  if (fix.manager.last_chat_id == 89) fix.manager.last_chat_id = 0;
  int before_auth_call = fix.mock_autoritized->call_autoritize;

  fix.app.handle_full(fix.req, fix.res);

  REQUIRE(fix.mock_autoritized->call_autoritize == before_auth_call + 1);
  REQUIRE(fix.manager.call_getChatById == before + 1);
  REQUIRE(fix.manager.last_chat_id == 89);
}

TEST_CASE(
    "handleGetAllChatsMembers listens on GET /chats/<int>/members and call "
    "NetworkManager::GetMembersChat with expected chat_id") {
  TestServer::TestFixture fix;

  SECTION("Token not setted expected call") {
    fix.mock_autoritized->need_fail = true;
    fix.app.validate();
    fix.req.method = "GET"_method;
    fix.req.url = "/chats/42/members";
    int before_getMembersCall = fix.network_manager.call_getMembersOfChat;
    if (fix.network_manager.last_chat_id == 42) fix.network_manager.last_chat_id = 0;
    int before_auth_call = fix.mock_autoritized->call_autoritize;

    fix.app.handle_full(fix.req, fix.res);

    REQUIRE(fix.mock_autoritized->call_autoritize == before_auth_call);
    REQUIRE(fix.manager.call_getMembersOfChat == before_getMembersCall + 1);
  }

  SECTION("Token is setted expected call") {
    fix.req.add_header("Authorization", fix.secret_token);
    fix.app.validate();
    fix.req.method = "GET"_method;
    fix.req.url = "/chats/42/members";
    int before_getMembersCall = fix.network_manager.call_getMembersOfChat;
    if (fix.network_manager.last_chat_id == 42) fix.network_manager.last_chat_id = 0;
    int before_auth_call = fix.mock_autoritized->call_autoritize;

    fix.app.handle_full(fix.req, fix.res);

    REQUIRE(fix.mock_autoritized->call_autoritize == before_auth_call);
    REQUIRE(fix.manager.call_getMembersOfChat == before_getMembersCall + 1);
    REQUIRE(fix.manager.last_chat_id == 42);
  }
}

TEST_CASE(
    "handleGetAllChatsUser listens on GET /chats and call "
    "Manager::getChatsOfUser with expected "
    "user_id") {
  TestServer::TestFixture fix;
  SECTION("Token isn't setted expected no call") {
    fix.mock_autoritized->need_fail = true;
    fix.app.validate();
    fix.req.method = "GET"_method;
    fix.req.url = "/chats";
    int before = fix.manager.call_getChatsOfUser;
    int user_id = 12;
    fix.mock_autoritized->mock_user_id = user_id;
    int before_auth_call = fix.mock_autoritized->call_autoritize;

    fix.app.handle_full(fix.req, fix.res);

    REQUIRE(fix.mock_autoritized->call_autoritize == before_auth_call + 1);
    REQUIRE(fix.mock_autoritized->last_token == "");
    REQUIRE(fix.manager.call_getChatsOfUser == before);
    REQUIRE(fix.res.code == Config::StatusCodes::userError);
    REQUIRE(fix.res.body == fix.formError(Config::IssueMessages::invalidToken));
  }

  SECTION("Token is setted expected call") {
    fix.app.validate();
    fix.req.method = "GET"_method;
    fix.req.url = "/chats";
    int before = fix.manager.call_getChatsOfUser;
    int user_id = 12;
    fix.req.add_header("Authorization", fix.secret_token);
    fix.mock_autoritized->mock_user_id = user_id;
    int before_auth_call = fix.mock_autoritized->call_autoritize;

    fix.app.handle_full(fix.req, fix.res);

    REQUIRE(fix.mock_autoritized->last_token == fix.secret_token);
    REQUIRE(fix.mock_autoritized->call_autoritize == before_auth_call + 1);
    REQUIRE(fix.manager.call_getChatsOfUser == before + 1);
    REQUIRE(fix.manager.last_user_id == user_id);
  }
}

TEST_CASE(
    "handleGetAllChatsUser on GET /chats after authentifiaction receive "
    "invalid token expected no "
    "call Manager::getChatsOfUser") {
  TestServer::TestFixture fix;
  fix.mock_autoritized->need_fail = true;
  fix.app.validate();
  fix.req.method = "GET"_method;
  fix.req.url = "/chats";
  int before = fix.manager.call_getChatsOfUser;
  int before_auth_call = fix.mock_autoritized->call_autoritize;

  fix.app.handle_full(fix.req, fix.res);

  REQUIRE(fix.mock_autoritized->call_autoritize == before_auth_call + 1);
  REQUIRE(fix.manager.call_getChatsOfUser == before);
  REQUIRE(fix.res.code == Config::StatusCodes::userError);
  REQUIRE(fix.res.body == fix.formError(Config::IssueMessages::invalidToken));
}
