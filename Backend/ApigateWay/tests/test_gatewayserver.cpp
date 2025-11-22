#include <catch2/catch_all.hpp>

#include "gatewayserver.h"
#include "mocks/MockProxy.h"
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
    MockProxy proxy;
    MockConfigProvider provider;
    crow::request req;
    crow::response res;
    TestGatewayServer server;
    std::string mock_in_cache_ans = "TEST CACHE HIT";

    TestGatewayServerFixrute()
        : server(app, &cache, &proxy, &provider) {
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
    int before_cnt_forward = fix.proxy.call_forward;

    fix.app.handle_full(fix.req, fix.res);

    REQUIRE(fix.proxy.call_forward == before_cnt_forward + 1);
    CHECK(fix.proxy.last_port == fix.provider.ports().authService);
    CHECK(fix.proxy.last_request.body == fix.req.body);
    CHECK(fix.proxy.last_request.method == fix.req.method);
    CHECK(fix.proxy.last_request.url == fix.req.url);
    CHECK(fix.proxy.last_request.url_params.keys() == fix.req.url_params.keys());
  }

  SECTION("Responce received expected no set cache(POST request) ans responce with same data") {
    int mock_code = 1234;
    std::string mock_body = "test_mock_body";
    fix.proxy.mock_response = std::make_pair(mock_code, mock_body);
    int before_cnt_cache = fix.cache.call_set;
    int before_cnt_forward = fix.proxy.call_forward;

    fix.app.handle_full(fix.req, fix.res);

    REQUIRE(fix.proxy.call_forward == before_cnt_forward + 1);
    REQUIRE(fix.res.code == mock_code);
    REQUIRE(fix.res.body == mock_body);
    REQUIRE(fix.cache.call_set == before_cnt_cache);
  }
}

TEST_CASE("Test apigate GET method") {
  TestGatewayServerFixrute fix;
  fix.app.validate();
  fix.req.method = "GET"_method;
  int chat_id = 12;
  fix.req.url = fmt::format("/messages/{}", chat_id);

  SECTION("Send forward request has to have valid port, method and request") {
    int before_forward_cnt = fix.proxy.call_forward;

    fix.app.handle_full(fix.req, fix.res);

    REQUIRE(fix.proxy.call_forward == before_forward_cnt + 1);
    CHECK(fix.proxy.last_port == fix.provider.ports().messageService);
    CHECK(fix.proxy.last_request.body == fix.req.body);
    CHECK(fix.proxy.last_request.method == fix.req.method);
    CHECK(fix.proxy.last_request.url == fix.req.url);
    CHECK(fix.proxy.last_request.url_params.keys() == fix.req.url_params.keys());
  }

  SECTION("Cache not hitted expected after responce set result in cache and return answer from forward") {
    int before_forward_cnt = fix.proxy.call_forward;
    int before_cnt_cache_set = fix.cache.call_set;
    int mock_code = 1234;
    std::string mock_body = "test_mock_body";
    fix.proxy.mock_response = std::make_pair(mock_code, mock_body);

    fix.app.handle_full(fix.req, fix.res);

    REQUIRE(fix.proxy.call_forward == before_forward_cnt + 1);
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
  fix.req.add_header("X-Forwarded-For", "123.45.67.89");
  fix.req.remote_ip_address = "98.76.54.32";

  std::string ip = fix.server.extractIP(fix.req);

  CHECK(ip == "123.45.67.89");
}

TEST_CASE("extractIP falls back to remote_ip_address") {
  TestGatewayServerFixrute fix;
  fix.req.remote_ip_address = "98.76.54.32";

  std::string ip = fix.server.extractIP(fix.req);

  CHECK(ip == "98.76.54.32");
}

TEST_CASE("extractToken extracts Bearer token") {
  TestGatewayServerFixrute fix;
  fix.req.add_header("Authorization", "Bearer abc123");

  std::string token = fix.server.extractToken(fix.req);

  CHECK(token == "abc123");
}

TEST_CASE("extractToken returns raw Authorization header when not Bearer") {
  TestGatewayServerFixrute fix;
  fix.req.add_header("Authorization", "Token xyz");

  std::string token = fix.server.extractToken(fix.req);

  CHECK(token == "Token xyz");
}

TEST_CASE("extractToken returns empty string when no Authorization header") {
  TestGatewayServerFixrute fix;

  std::string token = fix.server.extractToken(fix.req);

  CHECK(token.empty());
}

