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

TEST_CASE("Test apigate") {
  TestGatewayServerFixrute fix;

  SECTION("Check rate limit fails expected rateLimit code and rateLimitExceed message") {
    fix.server.shouldFail_check_limit = true;
    fix.app.validate();
    fix.req.method = "POST"_method;
    fix.req.url = "/auth/login";
    fix.req.body = R"({"email":"a","passwords":"b"})";

    fix.app.handle_full(fix.req, fix.res);

    REQUIRE(fix.res.code == fix.provider.statusCodes().rateLimit);
    REQUIRE(fix.res.body == fix.provider.issueMessages().rateLimitExceed);
  }

  SECTION("Check auth  fails expected unauthorized code and invalidToken message") {
    fix.server.shouldFail_check_auth = true;
    fix.app.validate();
    fix.req.method = "POST"_method;
    fix.req.url = "/auth/login";
    fix.req.body = R"({"email":"a","passwords":"b"})";

    fix.app.handle_full(fix.req, fix.res);

    REQUIRE(fix.res.code == fix.provider.statusCodes().unauthorized);
    REQUIRE(fix.res.body == fix.provider.issueMessages().invalidToken);
  }

  SECTION("Hit cache expected hit cnt increasy by one and success status code") {
    fix.cache.mock_answer = nlohmann::json(fix.mock_in_cache_ans);
    fix.app.validate();
    fix.req.method = "POST"_method;
    fix.req.url = "/auth/login";
    fix.req.body = R"({"email":"a","passwords":"b"})";
    int before_hit_key = fix.server.cnt_hit_key;

    fix.app.handle_full(fix.req, fix.res);

    REQUIRE(fix.server.cnt_hit_key == before_hit_key + 1);
    REQUIRE(fix.res.code == fix.provider.statusCodes().success);
    REQUIRE(fix.res.body == fix.mock_in_cache_ans);
  }

  SECTION("In any case send responce expected POST method, /auth/login path") {
    fix.app.validate();
    fix.req.method = "POST"_method;
    fix.req.url = "/auth/login";
    fix.req.body = R"({"email":"a","passwords":"b"})";
    int before_cnt_send_response = fix.server.cnt_sendResponde;

    fix.app.handle_full(fix.req, fix.res);

    REQUIRE(fix.server.cnt_sendResponde == before_cnt_send_response + 1);
    auto last_request_info = fix.server.last_request_info;
    CHECK(last_request_info.method == "POST");
    CHECK(last_request_info.path == "/auth/login");
  }

  SECTION("Cache not hitted expected send request to service with valid route ans same request") {
    fix.app.validate();
    fix.req.method = "POST"_method;
    fix.req.url = "/auth/login";
    fix.req.body = R"({"email":"a","passwords":"b"})";
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

    fix.app.validate();
    fix.req.method = "POST"_method;
    fix.req.url = "/auth/login";
    fix.req.body = R"({"email":"a","passwords":"b"})";
    int before_cnt_forward = fix.proxy.call_forward;

    fix.app.handle_full(fix.req, fix.res);

    REQUIRE(fix.proxy.call_forward == before_cnt_forward + 1);
    REQUIRE(fix.res.code == mock_code);
    REQUIRE(fix.res.body == mock_body);
    REQUIRE(fix.cache.call_set == before_cnt_cache);
  }
}
