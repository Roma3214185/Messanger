#include <catch2/catch_all.hpp>

#include "authservice/authcontroller.h"
#include "authservice/server.h"
#include "mocks/MockAutoritizer.h"
#include "mocks/MockIdGenerator.h"
#include "mocks/authservice/MockAuthManager.h"
#include "mocks/authservice/MockGenerator.h"
#include "config/codes.h"

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
  User user;
  std::string token = "secret-test-token";
  MockTokenGenerator generator;

  TestFixture() : controller(&manager, &authoritizer, &generator), server(app, port, &controller) {
    authoritizer.mock_user_id = user_id;
    user.id = user_id;
    user.username = "Test_username";
    user.tag = "test_tag";
    user.email = "test_email";
    user.avatar = "test/avatar/path";
    generator.mock_token = token;
    server.initRoutes();
  }

  std::string formError(const std::string &text) {
    nlohmann::json json;
    json["error"] = text;
    return json.dump();
  }
};

}  // namespace Test

TEST_CASE("handleMe listens on GET /auth/me") {
  Test::TestFixture fix;
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
    REQUIRE(fix.res.code == Config::StatusCodes::unauthorized);
    REQUIRE(fix.res.body == fix.formError(Config::IssueMessages::invalidToken));
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
    REQUIRE(fix.res.code == Config::StatusCodes::notFound);
    REQUIRE(fix.res.body == fix.formError(Config::IssueMessages::userNotFound));
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

    REQUIRE(fix.res.code == Config::StatusCodes::success);

    auto r = crow::json::load(fix.res.body);

    REQUIRE(r.size() == 2);
    CHECK(r["token"].s() == fix.token);
    CHECK(r["user"]["id"].i() == fix.user.id);
    CHECK(r["user"]["avatar"].s() == fix.user.avatar);
    CHECK(r["user"]["name"].s() == fix.user.username);
    CHECK(r["user"]["email"].s() == fix.user.email);
    CHECK(r["user"]["tag"].s() == fix.user.tag);
  }
}

TEST_CASE("handleLogin listens on POST /auth/login") {
  Test::TestFixture fix;
  SECTION("Request without body expected badRequest error") {
    fix.app.validate();
    fix.req.method = "POST"_method;
    fix.req.url = "/auth/login";

    fix.app.handle_full(fix.req, fix.res);

    CHECK(fix.res.code == Config::StatusCodes::badRequest);
    CHECK(fix.res.body == fix.formError("Invalid Json"));
  }

  SECTION("Request without email or password expected badRequest error") {
    fix.app.validate();
    fix.req.method = "POST"_method;
    fix.req.url = "/auth/login";
    fix.req.body = R"({"email":"a","passwords":"b"})";

    fix.app.handle_full(fix.req, fix.res);

    REQUIRE(fix.res.code == Config::StatusCodes::badRequest);
    REQUIRE(fix.res.body == fix.formError("Invalid Json"));
  }

  fix.req.body = R"({"email":"test_email","password":"test_password"})";

  SECTION("Manager not loggin user expected unauthorized error") {
    fix.app.validate();
    fix.req.method = "POST"_method;
    fix.req.url = "/auth/login";
    fix.manager.mock_user = std::nullopt;

    fix.app.handle_full(fix.req, fix.res);

    REQUIRE(fix.manager.last_login_request.email == "test_email");
    REQUIRE(fix.manager.last_login_request.password == "test_password");
    REQUIRE(fix.res.code == Config::StatusCodes::badRequest);
    REQUIRE(fix.res.body == fix.formError("Invalid credentials"));
  }

  SECTION("User log in expected success code and valid json") {
    fix.app.validate();
    fix.req.method = "POST"_method;
    fix.req.url = "/auth/login";
    fix.manager.mock_user = fix.user;

    fix.app.handle_full(fix.req, fix.res);

    REQUIRE(fix.res.code == Config::StatusCodes::success);

    auto r = crow::json::load(fix.res.body);
    REQUIRE(r.size() == 2);
    CHECK(r["token"].s() == fix.token);
    CHECK(r["user"]["id"].i() == fix.user.id);
    CHECK(r["user"]["avatar"].s() == fix.user.avatar);
    CHECK(r["user"]["name"].s() == fix.user.username);
    CHECK(r["user"]["email"].s() == fix.user.email);
    CHECK(r["user"]["tag"].s() == fix.user.tag);
  }
}

TEST_CASE("handleLogin listens on POST /auth/register") {
  Test::TestFixture fix;
  SECTION("Request without body expected badRequest error") {
    fix.app.validate();
    fix.req.method = "POST"_method;
    fix.req.url = "/auth/register";

    fix.app.handle_full(fix.req, fix.res);

    REQUIRE(fix.res.code == Config::StatusCodes::badRequest);
    REQUIRE(fix.res.body == fix.formError("Invalid json"));
  }

  SECTION("Request without password expected badRequest error") {
    fix.app.validate();
    fix.req.method = "POST"_method;
    fix.req.url = "/auth/register";
    fix.req.body = R"({"email":"a", "name":"a", "tag":"a"})";

    fix.app.handle_full(fix.req, fix.res);

    REQUIRE(fix.res.code == Config::StatusCodes::badRequest);
    REQUIRE(fix.res.body == fix.formError("Invalid json"));
  }

  fix.req.body = R"({"email":"a","password":"b", "name":"c", "tag":"d"})";

  SECTION("Valid register request expected no badRequest error") {
    fix.app.validate();
    fix.req.method = "POST"_method;
    fix.req.url = "/auth/register";

    fix.app.handle_full(fix.req, fix.res);

    REQUIRE(fix.res.body != fix.formError("Invalid json"));
  }

  SECTION("Manager not register user expected unauthorized error") {
    fix.app.validate();
    fix.req.method = "POST"_method;
    fix.req.url = "/auth/register";
    fix.manager.mock_user = std::nullopt;

    fix.app.handle_full(fix.req, fix.res);

    REQUIRE(fix.manager.last_register_request.email == "a");
    REQUIRE(fix.manager.last_register_request.password == "b");
    REQUIRE(fix.manager.last_register_request.name == "c");
    REQUIRE(fix.manager.last_register_request.tag == "d");
    REQUIRE(fix.res.code == Config::StatusCodes::userError);
    REQUIRE(fix.res.body == fix.formError("User already exist"));
  }

  SECTION("Valid request expected success code and valid json") {
    fix.app.validate();
    fix.req.method = "POST"_method;
    fix.req.url = "/auth/register";
    fix.manager.mock_user = fix.user;

    fix.app.handle_full(fix.req, fix.res);

    REQUIRE(fix.res.code == Config::StatusCodes::success);

    auto r = crow::json::load(fix.res.body);
    REQUIRE(r.size() == 2);
    CHECK(r["token"].s() == fix.token);
    CHECK(r["user"]["id"].i() == fix.user.id);
    CHECK(r["user"]["avatar"].s() == fix.user.avatar);
    CHECK(r["user"]["name"].s() == fix.user.username);
    CHECK(r["user"]["email"].s() == fix.user.email);
    CHECK(r["user"]["tag"].s() == fix.user.tag);
  }
}

TEST_CASE("findByTag listens GET users/search") {
  Test::TestFixture fix;
  SECTION(
      "Request without tag expected badRequest and Missing tag parametr "
      "error") {
    fix.app.validate();
    fix.req.method = "GET"_method;
    fix.req.url = "/users/search";

    fix.app.handle_full(fix.req, fix.res);

    REQUIRE(fix.res.code == Config::StatusCodes::badRequest);
    REQUIRE(fix.res.body == fix.formError("Missing tag parametr"));
  }

  SECTION("Request with tag expected request to manager with given tag") {
    fix.app.validate();
    fix.req.method = "GET"_method;
    fix.req.url = "/users/search";
    fix.req.url_params = crow::query_string("?tag=secret-tag");

    fix.app.handle_full(fix.req, fix.res);

    REQUIRE(fix.manager.last_tag == "secret-tag");
  }

  fix.req.url_params = crow::query_string("?tag=secret-tag");
  User user1;
  user1.id = 1;
  user1.username = "1name";
  user1.tag = "1tag";
  user1.email = "1email";
  user1.avatar = "1avatar/path";
  User user2;
  user2.id = 2;
  user2.username = "2name";
  user2.tag = "2tag";
  user2.email = "2email";
  user2.avatar = "2avatar/path";
  fix.manager.mock_users = {user1, user2};

  SECTION("Valid request expected success code and json") {
    fix.app.validate();
    fix.req.method = "GET"_method;
    fix.req.url = "/users/search";

    fix.app.handle_full(fix.req, fix.res);

    REQUIRE(fix.res.code == Config::StatusCodes::success);

    auto result = crow::json::load(fix.res.body);
    REQUIRE(result["users"].size() == 2);

    REQUIRE(result["users"][0]["id"].i() == 1);
    REQUIRE(result["users"][0]["email"].s() == "1email");
    REQUIRE(result["users"][0]["name"].s() == "1name");
    REQUIRE(result["users"][0]["tag"].s() == "1tag");
    REQUIRE(result["users"][0]["avatar"].s() == "1avatar/path");

    REQUIRE(result["users"][1]["id"].i() == 2);
    REQUIRE(result["users"][1]["email"].s() == "2email");
    REQUIRE(result["users"][1]["name"].s() == "2name");
    REQUIRE(result["users"][1]["tag"].s() == "2tag");
    REQUIRE(result["users"][1]["avatar"].s() == "2avatar/path");
  }
}

TEST_CASE("handleFindById listens /users/<int>") {
  Test::TestFixture fix;
  fix.req.url = "/users/123";

  SECTION("User not found expected notFound code and User not found message") {
    fix.app.validate();
    fix.req.method = "GET"_method;
    fix.manager.mock_user = std::nullopt;

    fix.app.handle_full(fix.req, fix.res);

    REQUIRE(fix.manager.last_user_id == 123);
    REQUIRE(fix.res.code == Config::StatusCodes::notFound);
    REQUIRE(fix.res.body == fix.formError("User not found"));
  }

  User user;
  user.id = 123;
  user.username = "3name";
  user.tag = "3tag";
  user.email = "3email";
  user.avatar = "3avatar/path";
  fix.manager.mock_user = user;

  SECTION("User found expected success code and valid json message") {
    fix.app.validate();
    fix.req.method = "GET"_method;

    fix.app.handle_full(fix.req, fix.res);

    REQUIRE(fix.manager.last_user_id == 123);
    REQUIRE(fix.res.code == Config::StatusCodes::success);
    auto result = crow::json::load(fix.res.body);
    REQUIRE(result.size() == 5);
    CHECK(result["id"].i() == 123);
    CHECK(result["email"].s() == "3email");
    CHECK(result["name"].s() == "3name");
    CHECK(result["tag"].s() == "3tag");
    CHECK(result["avatar"].s() == "3avatar/path");
  }
}
