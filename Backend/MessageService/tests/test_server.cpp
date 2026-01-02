// #include <catch2/catch_all.hpp>
// #include <crow.h>

// #include "messageservice/server.h"
// #include "messageservice/controller.h"
// #include "mocks/MockConfigProvider.h"

// struct TestFixture{
//   crow::SimpleApp app;
//   int port = 101;
//   Controller controller;
//   MockConfigProvider provider;
//   crow::request req;
//   crow::response res;
//   int chat_id = 44;
//   TestFixture() {
//     //StatusCodes mock_codes;
//     // mock_codes.serverError = 1;
//     // mock_codes.success = 2;
//     // mock_codes.userError = 3;
//     // provider.mock_codes = mock_codes;

//     //provider.mock_issue_message.invalidToken = "test_token is invalid";
//   }
// };

// class TestServer : public Server {
//   public:
//     using Server::Server;
//     int calls_onGetMessagesFromChat = 0;
//     int last_chat_id;

//     void onGetMessagesFromChat(const crow::request& req, long long chat_id, crow::response& res) override {
//       ++calls_onGetMessagesFromChat;
//       last_chat_id = chat_id;
//     }
// };

// TEST_CASE("Server listens on /messages/<int> and calls onGetMessagesFromChat") {
//   TestFixture fix;
//   TestServer server(fix.app, fix.port, &fix.controller, &fix.provider);

//   int before = server.calls_onGetMessagesFromChat;
//   fix.app.validate();
//   fix.req.method = "GET"_method;
//   fix.req.url = fmt::format("/messages/{}", fix.chat_id);

//   fix.app.handle_full(fix.req, fix.res);

//   REQUIRE(server.calls_onGetMessagesFromChat == before + 1);
//   REQUIRE(server.last_chat_id == fix.chat_id);
// }

// struct TestServer2 : public Server {
//   public:
//     using Server::Server;
//     using Server::formMessageListJson;
//     std::string last_token;
//     std::optional<long long> mock_user_id_ans;
//     int calls_getUserIdFromToken = 0;

//     std::optional<long long> getUserIdFromToken(const std::string &token) override {
//       ++calls_getUserIdFromToken;
//       last_token = token;
//       return mock_user_id_ans;
//     }
// };

// TEST_CASE("Test OngetMessagesFromChatCallRight Fucntion") {
//   TestFixture fix;
//   TestServer2 server(fix.app, fix.port, &fix.controller, &fix.provider);

//   fix.app.validate();
//   fix.req.method = "GET"_method;
//   fix.req.url = fmt::format("/messages/{}", fix.chat_id);
//   std::string secret_token = "secret_token";

//   SECTION("Request without token expected last token is empty string") {
//     int before_calls = server.calls_getUserIdFromToken;
//     fix.app.handle_full(fix.req, fix.res);
//     REQUIRE(server.calls_getUserIdFromToken == before_calls + 1);
//     REQUIRE(server.last_token.empty());
//   }

//   fix.req.add_header("Authorization", secret_token);

//   SECTION("Request with token expected last token is setted token") {
//     int before_calls = server.calls_getUserIdFromToken;
//     fix.app.handle_full(fix.req, fix.res);
//     REQUIRE(server.calls_getUserIdFromToken == before_calls + 1);
//     REQUIRE(server.last_token == secret_token);
//   }

//   SECTION("Token invalid or expired expected return error code and test about problem") {
//     server.mock_user_id_ans = std::nullopt;
//     fix.app.handle_full(fix.req, fix.res);

//     REQUIRE(fix.res.code == fix.provider.statusCodes().userError);
//     REQUIRE(fix.res.body == fix.provider.issueMessages().invalidToken);
//   }

//   int user_id = 12;
//   server.mock_user_id_ans = user_id;

//   SECTION("Fucntion get Valid data from request") {
//     fix.req.url_params = crow::query_string("?limit=21&before_id=123");
//     fix.app.handle_full(fix.req, fix.res);

//     auto last_pack = fix.controller.mock_messages_pack;
//     CHECK(last_pack.before_id == 123);
//     CHECK(last_pack.limit == 21);
//     CHECK(last_pack.user_id == user_id);
//     CHECK(last_pack.chat_id == fix.chat_id);
//   }

//   SECTION("Fucntion without any url_params create valid request") {
//     fix.app.handle_full(fix.req, fix.res);

//     auto last_pack = fix.controller.mock_messages_pack;
//     CHECK(last_pack.before_id == 0);
//     CHECK(last_pack.limit == INT_MAX);
//     CHECK(last_pack.user_id == user_id);
//     CHECK(last_pack.chat_id == fix.chat_id);
//   }

//   SECTION("Get message status expected return success codes") {
//     fix.app.handle_full(fix.req, fix.res);
//     REQUIRE(fix.res.code == fix.provider.statusCodes().success);
//   }
// }

// TEST_CASE("formMessageListJson creates correct JSON array") {
//   TestFixture fix;
//   TestServer2 server(fix.app, fix.port, &fix.controller, &fix.provider);

//   std::vector<Message> messages = {
//       {.id = 1, .text = "hello"},
//       {.id = 2, .text = "world"}
//   };

//   std::vector<MessageStatus> statuses = {
//       {.is_read = true},
//       {.is_read = false}
//   };

//   auto json = server.formMessageListJson(messages, statuses);
//   auto r = crow::json::load(json.dump());

//   REQUIRE(r.size() == 2);
//   CHECK(r[0]["id"].i() == 1);
//   CHECK(r[0]["text"].s() == "hello");
//   CHECK(r[0]["readed_by_me"].b() == true);

//   CHECK(r[1]["id"].i() == 2);
//   CHECK(r[1]["text"].s() == "world");
//   CHECK(r[1]["readed_by_me"].b() == false);
// }

// TEST_CASE("formMessageListJson got vectors of different sizes expected return empty JSON array") {
//   TestFixture fix;
//   TestServer2 server(fix.app, fix.port, &fix.controller, &fix.provider);

//   std::vector<Message> messages = {
//       {.id = 1, .text = "hello"},
//       {.id = 2, .text = "world"}
//   };

//   std::vector<MessageStatus> statuses = {
//       {.is_read = true},
//   };

//   auto json = server.formMessageListJson(messages, statuses);
//   auto r = crow::json::load(json.dump());

//   REQUIRE(r.size() == 0);
// }

