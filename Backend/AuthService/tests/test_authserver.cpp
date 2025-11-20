#include <catch2/catch_all.hpp>

#include "authservice/server.h"
#include "authservice/authcontroller.h"
#include "mocks/MockAuthManager.h"
#include "mocks/MockAutoritizer.h"
#include "mocks/MockConfigProvider.h"
#include "mocks/MockUtils.h"

namespace Test {

struct TestFixture {
    crow::SimpleApp app;
    MockAuthManager manager;
    int port = 100;
    std::string secret_token = "Secret-token-123";
    crow::request req;
    crow::response res;
    int user_id = 13;
    Server server;
    AuthController controller;
    MockAutoritizer authoritizer;
    MockConfigProvider provider;
    User user;
    std::string token = "secret-test-token";

    TestFixture()
        : controller(&manager, &authoritizer, &provider)
        , server(app, port, &controller) {
      authoritizer.mock_user_id = user_id;
      provider.mock_codes =  MockUtils::getMockCodes();
      user.id = user_id;
      user.username = "Test_username";
      user.tag = "test_tag";
      user.email = "test_email";
      user.avatar = "test/avatar/path";
    }
};

}  // namespace TestServer

TEST_CASE("handleMe listens on GET /auth/me") {
  Test::TestFixture fix;

  // void AuthController::handleMe(const crow::request& req, crow::response& responce) {
  //   auto [user_id, token] = verifyToken(req);
  //   if (!user_id) {
  //     return sendResponse(responce, provider_->statusCodes().unauthorized, provider_->statusCodes().invalidToken);
  //   }

  //   std::optional<User> user = manager_->getUser(*user_id);
  //   if(!user) {
  //     return sendResponse(responce, provider_->statusCodes().notFound, "User not found");
  //   }

  //   sendResponse(responce, provider_->statusCodes().success, userToJson(*user, token).dump());
  // }

  SECTION("Invalid token expected ") {
    fix.authoritizer.need_fail = true;
    fix.app.validate();
    fix.req.method = "GET"_method;
    fix.req.url = "/auth/me";
    int before_auth_call = fix.authoritizer.call_autoritize;
    int before_call_manager = fix.manager.call_getUser;

    fix.app.handle_full(fix.req, fix.res);

    REQUIRE(fix.authoritizer.call_autoritize == before_auth_call + 1);
    REQUIRE(fix.manager.call_getUser == before_call_manager);
    REQUIRE(fix.res.code == fix.provider.statusCodes().unauthorized);
    REQUIRE(fix.res.body == fix.provider.statusCodes().invalidToken);
  }
  fix.req.add_header("Authorization", fix.token);

  SECTION("User not found expected notFound error code") {
    fix.app.validate();
    fix.req.method = "GET"_method;
    fix.req.url = "/auth/me";
    int before_auth_call = fix.authoritizer.call_autoritize;
    int before_call_manager = fix.manager.call_getUser;
    fix.manager.mock_user = std::nullopt;
    fix.authoritizer.mock_user_id = fix.user_id;

    fix.app.handle_full(fix.req, fix.res);

    REQUIRE(fix.authoritizer.call_autoritize == before_auth_call + 1);
    REQUIRE(fix.manager.call_getUser == before_call_manager + 1);
    REQUIRE(fix.authoritizer.last_token == fix.token);
    REQUIRE(fix.manager.last_user_id == fix.user_id);
    REQUIRE(fix.res.code == fix.provider.statusCodes().notFound);
    REQUIRE(fix.res.body == "User not found");
  }

  SECTION("Expected return valid status code and form json") {
    fix.app.validate();
    fix.req.method = "GET"_method;
    fix.req.url = "/auth/me";
    int before_auth_call = fix.authoritizer.call_autoritize;
    int before_call_manager = fix.manager.call_getUser;
    fix.manager.mock_user = fix.user;
    fix.authoritizer.mock_user_id = fix.user.id;

    fix.app.handle_full(fix.req, fix.res);

    REQUIRE(fix.res.code == fix.provider.statusCodes().success);

    auto r = crow::json::load(fix.res.body);

    REQUIRE(r.size() == 2);
    CHECK(r["token"].s() == fix.token);
    CHECK(r["user"]["id"].i()   == fix.user.id);
    CHECK(r["user"]["avatar"].s()  == fix.user.avatar);
    CHECK(r["user"]["name"].s() == fix.user.username);
    CHECK(r["user"]["email"].s() == fix.user.email);
    CHECK(r["user"]["tag"].s() == fix.user.tag);
  }


  // SECTION("Invalid token expected not call getChatsOfUser") {
  //   fix.mock_autoritized->need_fail = true;
  //   fix.app.validate();
  //   fix.req.method = "GET"_method;
  //   fix.req.url = "/chats";
  //   int before = fix.manager.call_getChatsOfUser;

  //   fix.app.handle_full(fix.req, fix.res);

  //   REQUIRE(fix.manager.call_getChatsOfUser == before);
  //   REQUIRE(fix.res.code == fix.provider.statusCodes().userError);
  //   REQUIRE(fix.res.body == fix.provider.statusCodes().invalidToken);
  // }

  // fix.req.add_header("Authorization", fix.secret_token);

  // SECTION("Token is setted expected call getChatsOfUser") {
  //   fix.app.validate();
  //   fix.req.method = "GET"_method;
  //   fix.req.url = "/chats";
  //   int before_getChatCall = fix.manager.call_getChatsOfUser;
  //   int before_auth_call = fix.mock_autoritized->call_autoritize;

  //   fix.app.handle_full(fix.req, fix.res);

  //   REQUIRE(fix.mock_autoritized->call_autoritize == before_auth_call + 1);
  //   REQUIRE(fix.mock_autoritized->last_token == fix.secret_token);
  //   REQUIRE(fix.manager.call_getChatsOfUser == before_getChatCall + 1);
  //   REQUIRE(fix.manager.last_user_id == fix.user_id);
  // }
}


