#include <catch2/catch_all.hpp>

#include "gatewayserver.h"
#include "mocks/MockClient.h"
#include "mocks/MockConfigProvider.h"
#include "mocks/MockUtils.h"
#include "mocks/MockApiCache.h"

class TestGatewayServer : public GatewayServer {
  public:
    using GatewayServer::GatewayServer;
    crow::request last_check_limit_request;
    int call_checkRateLimit  = 0;
    bool shouldFail_check_limit = false;

    bool last_requireAuth;
    int call_checkAuth  = 0;
    bool shouldFail_check_auth = false;

    bool checkRateLimit(const crow::request& req) {
      last_check_limit_request = req;
      ++call_checkRateLimit;
      return !shouldFail_check_limit;
    }

    bool checkAuth(const crow::request &req, bool requireAuth) {
      ++call_checkAuth;
      return !shouldFail_check_auth;
    }

    RequestDTO last_request_info;
    int cnt_hit_key = 0;
    int cnt_sendResponde = 0;

    virtual void sendResponse(crow::response& res, const RequestDTO& request_info, int res_code, const std::string& message, bool hitKey = false) override {
      ++cnt_sendResponde;
      last_request_info = request_info;
      if(hitKey) ++cnt_hit_key;
      GatewayServer::sendResponse(res, request_info, res_code, message, hitKey);
    }

    std::string extractToken(const crow::request& req) const {
      return GatewayServer::extractToken(req);
    }
    std::string extractIP(const crow::request& req) const {
      return GatewayServer::extractIP(req);
    }
};

struct TestGatewayServerFixrute {
    crow::SimpleApp app;
    MockApiCache cache;
    MockClient client;
    MockConfigProvider provider;
    crow::request req;
    crow::response res;
    TestGatewayServer server;
    std::string mock_in_cache_ans = "TEST CACHE HIT";

    TestGatewayServerFixrute()
        : server(app, &cache, &client, &provider) {
      provider.mock_codes.rateLimit = 4290;
      provider.mock_codes.unauthorized = 4010;
      provider.mock_issue_message.rateLimitExceed = "test_rate_limit";
      provider.mock_issue_message.invalidToken = "test_invalid_token";
    }
};

TEST_CASE("Test apigate POST method") {
  TestGatewayServerFixrute fix;
  fix.app.validate();
  fix.req.method = "POST"_method;
  fix.req.url = "/auth/login";
  fix.req.body = R"({"email":"a","passwords":"b"})";

  SECTION("Check rate limit fails expected rateLimit code and rateLimitExceed message") {
    fix.server.shouldFail_check_limit = true;

    fix.app.handle_full(fix.req, fix.res);

    REQUIRE(fix.res.code == fix.provider.statusCodes().rateLimit);
    REQUIRE(fix.res.body == fix.provider.issueMessages().rateLimitExceed);
  }

  SECTION("Check auth  fails expected unauthorized code and invalidToken message") {
    fix.server.shouldFail_check_auth = true;

    fix.app.handle_full(fix.req, fix.res);

    REQUIRE(fix.res.code == fix.provider.statusCodes().unauthorized);
    REQUIRE(fix.res.body == fix.provider.issueMessages().invalidToken);
  }

  SECTION("Hit cache expected hit cnt increasy by one and success status code") {
    fix.cache.mock_answer = nlohmann::json(fix.mock_in_cache_ans);
    int before_hit_key = fix.server.cnt_hit_key;

    fix.app.handle_full(fix.req, fix.res);

    REQUIRE(fix.server.cnt_hit_key == before_hit_key + 1);
    REQUIRE(fix.res.code == fix.provider.statusCodes().success);
    REQUIRE(fix.res.body == fix.mock_in_cache_ans);
  }

  SECTION("In any case send responce expected POST method, /auth/login path") {
    fix.app.validate();
    int before_cnt_send_response = fix.server.cnt_sendResponde;

    fix.app.handle_full(fix.req, fix.res);

    REQUIRE(fix.server.cnt_sendResponde == before_cnt_send_response + 1);
    auto last_request_info = fix.server.last_request_info;
    CHECK(last_request_info.method == "POST");
    CHECK(last_request_info.path == "/auth/login");
  }

  SECTION("Cache not hitted expected send request to service with valid route ans same request") {
    int before_cnt_forward = fix.client.call_post;

    fix.app.handle_full(fix.req, fix.res);

    REQUIRE(fix.client.call_post == before_cnt_forward + 1);
    CHECK(fix.client.last_request.host_with_port == "localhost:" + std::to_string(fix.provider.ports().authService));
    CHECK(fix.client.last_request.body == fix.req.body);
    CHECK(fix.client.last_request.full_path == fix.req.url);
  }

  SECTION("Responce received expected no set cache(POST request) ans responce with same data") {
    int mock_code = 1234;
    std::string mock_body = "test_mock_body";
    fix.client.mock_response = std::make_pair(mock_code, mock_body);
    int before_cnt_cache = fix.cache.call_set;
    int before_cnt_forward = fix.client.call_post;

    fix.app.handle_full(fix.req, fix.res);

    REQUIRE(fix.client.call_post == before_cnt_forward + 1);
    REQUIRE(fix.res.code == mock_code);
    REQUIRE(fix.res.body == mock_body);
    REQUIRE(fix.cache.call_set == before_cnt_cache);
  }
}

TEST_CASE("Test apigate GET method") {
  TestGatewayServerFixrute fix;
  fix.req.body = "";
  fix.app.validate();
  fix.req.method = "GET"_method;
  int chat_id = 12;
  fix.req.url = fmt::format("/messages/{}", chat_id);

  SECTION("Send forward request has to have valid port, method and request") {
    int before_forward_cnt = fix.client.call_get;

    fix.app.handle_full(fix.req, fix.res);

    REQUIRE(fix.client.call_get == before_forward_cnt + 1);
    CHECK(fix.client.last_request.host_with_port == "localhost:" + std::to_string(fix.provider.ports().messageService));
    CHECK(fix.client.last_request.body == fix.req.body);
    CHECK(fix.client.last_request.full_path == fix.req.url);
  }

  SECTION("Cache not hitted expected after responce set result in cache and return answer from forward") {
    int before_forward_cnt = fix.client.call_get;
    int before_cnt_cache_set = fix.cache.call_set;
    int mock_code = 1234;
    std::string mock_body = "test_mock_body";
    fix.client.mock_response = std::make_pair(mock_code, mock_body);

    fix.app.handle_full(fix.req, fix.res);

    REQUIRE(fix.client.call_get == before_forward_cnt + 1);
    REQUIRE(fix.cache.call_set == before_cnt_cache_set + 1);
    REQUIRE(fix.res.code == mock_code);
    REQUIRE(fix.res.body == mock_body);
  }
}

TEST_CASE("Test apigate healthz endpoint") {
  TestGatewayServerFixrute fix;
  fix.app.validate();
  fix.req.method = "GET"_method;
  fix.req.url = "/healthz";

  fix.app.handle_full(fix.req, fix.res);

  auto r = crow::json::load(fix.res.body);

  CHECK(r["status"].s() == "ok");
  long long ts = r["timestamp"].i();
  long long now = std::chrono::duration_cast<std::chrono::milliseconds>(
                      std::chrono::system_clock::now().time_since_epoch())
                      .count();

  CHECK(std::llabs(now - ts) < 1000);
  CHECK(fix.res.get_header_value("Content-Type") == "application/json");
}

TEST_CASE("extractIP returns X-Forwarded-For when present") {
  TestGatewayServerFixrute fix;
  std::string setted_ip = "123.45.67.89";
  fix.req.add_header("X-Forwarded-For", setted_ip);
  fix.req.remote_ip_address = "123.21.34.34";

  std::string ip = fix.server.extractIP(fix.req);

  CHECK(ip == setted_ip);
}

TEST_CASE("extractIP falls back to remote_ip_address") {
  TestGatewayServerFixrute fix;
  std::string setted_ip = "98.76.54.32";
  fix.req.remote_ip_address = setted_ip;

  std::string ip = fix.server.extractIP(fix.req);

  CHECK(ip == setted_ip);
}

TEST_CASE("extractToken extracts Bearer token") {
  TestGatewayServerFixrute fix;
  fix.req.add_header("Authorization", "Bearer abc123");

  std::string token = fix.server.extractToken(fix.req);

  CHECK(token == "abc123");
}

TEST_CASE("extractToken returns raw Authorization header when not Bearer") {
  TestGatewayServerFixrute fix;
  std::string setted_token = "Token xyz";
  fix.req.add_header("Authorization", setted_token);

  std::string token = fix.server.extractToken(fix.req);

  CHECK(token == setted_token);
}

TEST_CASE("extractToken returns empty string when no Authorization header") {
  TestGatewayServerFixrute fix;

  std::string token = fix.server.extractToken(fix.req);

  CHECK(token.empty());
}

