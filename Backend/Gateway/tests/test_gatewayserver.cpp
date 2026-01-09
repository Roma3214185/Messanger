#include <catch2/catch_all.hpp>

#include "gatewayserver.h"
#include "mocks/MockRabitMQClient.h"
#include "mocks/MockTheadPool.h"
#include "mocks/gateway/GatewayMocks.h"
#include "config/ports.h"

struct TestGatewayServerFixrute {
  GatewayApp app;
  MockApiCache cache;
  MockClient client;
  crow::request req;
  crow::response res;
  GatewayServer server;
  MockMetrics metrics;
  std::string mock_client_ans = "TEST FORWARD";
  int mock_client_code = 1356;
  MockVerifier verifier;
  MockRateLimiter rate_limiter;
  MockThreadPool pool;
  MockRabitMQClient rabiq_client;
  int user_id = 123;

  TestGatewayServerFixrute()
      : server(app, &client, &cache, &pool, &rabiq_client) {
    app.get_middleware<AuthMiddleware>().verifier_ = &verifier;
    app.get_middleware<CacheMiddleware>().cache_ = &cache;
    app.get_middleware<LoggingMiddleware>();
    app.get_middleware<RateLimitMiddleware>().rate_limiter_ = &rate_limiter;
    app.get_middleware<MetricsMiddleware>().metrics_ = &metrics;

    verifier.mock_ans = user_id;
    client.mock_response = std::make_pair(mock_client_code, mock_client_ans);

    server.registerRoutes();
    app.validate();
  }

  void makeCall() {
    app.validate();
    app.handle_full(req, res);
  }
};

TEST_CASE("Test apigate POST method") {
  TestGatewayServerFixrute fix;
  fix.req.method = "POST"_method;
  fix.req.url = "/auth/login";
  fix.req.body = R"({"email":"a","passwords":"b"})";

  SECTION("Check server works with rabiq_mq and right create path") {
    int before_call_publish = fix.rabiq_client.publish_cnt;

    fix.makeCall();

    REQUIRE(fix.client.last_request.full_path == "/auth/login");
    REQUIRE(fix.rabiq_client.publish_cnt == before_call_publish + 1);
  }

  SECTION("Expected server call Post proxy with right port") {
    int before_post_calls = fix.client.call_post;

    fix.makeCall();

    REQUIRE(fix.client.call_post == before_post_calls + 1);
    REQUIRE(fix.client.last_request.host_with_port == "localhost:" + std::to_string(Config::Ports::authService));
  }

  SECTION(
      "Response received from client expected responce with 202 status "
      "code and request_id") {
    fix.makeCall();

    REQUIRE(fix.res.code == 202);  // TODO: make 202 in provider and extract generatorRequestId
    // REQUIRE(fix.res.body == fix.mock_client_ans);
  }
}

TEST_CASE("Test apigate GET method") {
  TestGatewayServerFixrute fix;
  fix.req.method = "GET"_method;
  int chat_id = 12;
  fix.req.url = fmt::format("/messages/{}", chat_id);

  SECTION("Send forward request has to have valid port, method and request") {
    int before_forward_cnt = fix.client.call_get;

    fix.makeCall();

    REQUIRE(fix.client.call_get == before_forward_cnt + 1);
    CHECK(fix.client.last_request.host_with_port == "localhost:" + std::to_string(Config::Ports::messageService));
    CHECK(fix.client.last_request.body == fix.req.body);
    CHECK(fix.client.last_request.full_path == fix.req.url);
  }

  SECTION("Returned results from client expectes return same repsonse") {
    int before_forward_cnt = fix.client.call_get;

    fix.makeCall();

    REQUIRE(fix.client.call_get == before_forward_cnt + 1);
    REQUIRE(fix.res.code == fix.mock_client_code);
    REQUIRE(fix.res.body == fix.mock_client_ans);
  }
}

TEST_CASE("Test simple base_path request") {
  TestGatewayServerFixrute fix;
  fix.req.method = "GET"_method;
  fix.req.url = "/chats";

  // SECTION("Check server enqueue work in pool") {
  //   int before_call_pool = fix.pool.call_count;

  //   fix.makeCall();

  //   REQUIRE(fix.pool.call_count == before_call_pool + 1);
  // }

  SECTION("Expected server call Post proxy with right port") {
    int before_post_calls = fix.client.call_get;

    fix.makeCall();

    REQUIRE(fix.client.call_get == before_post_calls + 1);
    REQUIRE(fix.client.last_request.host_with_port == "localhost:" + std::to_string(Config::Ports::chatService));
  }
}

TEST_CASE("Test apigate healthz endpoint") {
  TestGatewayServerFixrute fix;
  fix.app.validate();
  fix.req.method = "GET"_method;
  fix.req.url = "/healthz";

  fix.makeCall();

  auto r = crow::json::load(fix.res.body);

  CHECK(r["status"].s() == "ok");
  long long ts = r["timestamp"].i();
  long long now =
      std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
          .count();

  CHECK(std::llabs(now - ts) < 1000);
  CHECK(fix.res.get_header_value("Content-Type") == "application/json");
}
