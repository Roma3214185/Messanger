#include <catch2/catch_all.hpp>
#include "chatservice/chatserver.h"
#include "chatservice/chatcontroller.h"
#include "mocks/MockChatManager.h"
#include "mocks/MockNetworkManager.h"
#include "mocks/MockConfigProvider.h"
#include "NetworkFacade.h"
#include "chatservice/AutoritizerProvider.h"

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
    std::shared_ptr<MockAutoritizer> mock_autoritized;

    TestFixture()
      : controller(&manager, &facade, &provider)
      , server(app, port, &controller) {
      mock_autoritized = std::make_shared<MockAutoritizer>();
      AutoritizerProvider::set(mock_autoritized);
    }
};

TEST_CASE("handleCreatingPrivateChat listens on POST /chats/private and call Manager::GetAChatsOfUser") {
  TestFixture fix;
  fix.app.validate();
  fix.req.method = "GET"_method;
  fix.req.url = "/chats";
  int before = fix.manager.call_getChatsOfUser;

  fix.app.handle_full(fix.req, fix.res);

  REQUIRE(fix.manager.call_getChatsOfUser == before + 1);
}

TEST_CASE("handleGetChat listens on GET /chats/<int> and call Manager::GetChat with expected chat_id") {
  TestFixture fix;
  fix.app.validate();
  fix.req.method = "GET"_method;
  fix.req.url = "/chats/89";
  int before = fix.manager.call_getChatById;
  if(fix.manager.last_chat_id == 89) fix.manager.last_chat_id = 0;

  fix.app.handle_full(fix.req, fix.res);

  REQUIRE(fix.manager.call_getChatById == before + 1);
  REQUIRE(fix.manager.last_chat_id == 89);
}

TEST_CASE("handleGetAllChatsMembers listens on GET /chats/<int>/members and call NetworkManager::GetMembersChat with expected chat_id") {
  TestFixture fix;
  fix.app.validate();
  fix.req.method = "GET"_method;
  fix.req.url = "/chats/42/members";
  int before = fix.network_manager.call_getMembersOfChat;
  if(fix.network_manager.last_chat_id == 42) fix.network_manager.last_chat_id = 0;

  fix.app.handle_full(fix.req, fix.res);

  REQUIRE(fix.network_manager.call_getMembersOfChat == before + 1);
  REQUIRE(fix.network_manager.last_chat_id == 42);
}



TEST_CASE("handleGetAllChatsUser listens on GET /chats and call Manager::getChatsOfUser with expected user_id") {
  TestFixture fix;
  fix.app.validate();
  fix.req.method = "GET"_method;
  fix.req.url = "/chats";
  int before = fix.manager.call_getChatsOfUser;
  int user_id = 12;
  fix.mock_autoritized->mock_user_id = user_id;

  fix.app.handle_full(fix.req, fix.res);

  REQUIRE(fix.manager.call_getChatsOfUser == before + 1);
  REQUIRE(fix.manager.last_user_id == user_id);
}

TEST_CASE("handleGetAllChatsUser on GET /chats after authentifiaction receive nullopt expected no call Manager::getChatsOfUser") {
  TestFixture fix;
  fix.app.validate();
  fix.req.method = "GET"_method;
  fix.req.url = "/chats";
  int before = fix.manager.call_getChatsOfUser;
  fix.mock_autoritized->mock_user_id = std::nullopt;

  fix.app.handle_full(fix.req, fix.res);

  REQUIRE(fix.manager.call_getChatsOfUser == before);
}
