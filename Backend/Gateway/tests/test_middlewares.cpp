#include <crow.h>

#include <catch2/catch_all.hpp>

#include "middlewares/Middlewares.h"
#include "mocks/MockTheadPool.h"
#include "mocks/gateway/GatewayMocks.h"

struct DummyParentCtx {
  MetricsMiddleware::context metrics_ctx;

  template <typename MW>
  auto &get() {
    static_assert(std::is_same_v<MW, MetricsMiddleware>, "Only MetricsMiddleware supported in this dummy");
    return metrics_ctx;
  }
};

struct TestGatewayMiddlewaresFixrute {
  LoggingMiddleware logging_middleware;
  RateLimitMiddleware rate_limit_middleware;
  AuthMiddleware auth_middleware;
  CacheMiddleware cache_middleware;
  MetricsMiddleware metrics_middleware;
  MockApiCache cache;
  crow::request req;
  crow::response res;
  MockMetrics metrics;
  MockVerifier verifier;
  MockRateLimiter rate_limiter;
  DummyParentCtx dummy_parent_ctx;
  MockThreadPool pool;
  int user_id = 123;

  RateLimitMiddleware::context rate_ctx;
  AuthMiddleware::context auth_ctx;
  LoggingMiddleware::context log_ctx;
  CacheMiddleware::context cache_ctx;
  MetricsMiddleware::context metrics_ctx;

  TestGatewayMiddlewaresFixrute() {
    auth_middleware.verifier_ = &verifier;
    cache_middleware.cache_ = &cache;
    rate_limit_middleware.rate_limiter_ = &rate_limiter;
    metrics_middleware.metrics_ = &metrics;

    rate_limiter.should_fail = true;
    verifier.mock_ans = std::nullopt;
    cache.mock_answer = std::nullopt;
  }
};

TEST_CASE("Test Authmiddleware") {
  TestGatewayMiddlewaresFixrute fix;
  fix.req.url = "/auth/me";  // url that needs verification

  auto doCallBefore = [&]() {
    fix.auth_middleware.before_handle(fix.req, fix.res, fix.auth_ctx, fix.dummy_parent_ctx);
  };

  auto doCallAfter = [&]() { fix.auth_middleware.after_handle(fix.req, fix.res, fix.auth_ctx, fix.dummy_parent_ctx); };

  SECTION("Url that no need auth expected return") {
    fix.req.url = "/auth/login";
    doCallBefore();
    REQUIRE_FALSE(fix.res.is_completed());
  }

  SECTION("Token verified expected return") {
    fix.verifier.mock_ans = fix.user_id;
    doCallBefore();
    REQUIRE_FALSE(fix.res.is_completed());
  }

  SECTION(
      "Token not verifies expected unauthorized code and message about "
      "invalid token") {
    doCallBefore();
    REQUIRE(fix.res.is_completed());
    REQUIRE(fix.res.code == StatusCodes::unauthorized);
    REQUIRE(fix.res.body == IssueMessages::invalidToken);
  }

  SECTION("After handle expected no throw") { REQUIRE_NOTHROW(doCallAfter()); }
}

TEST_CASE("Test RateLimitMiddleware") {
  TestGatewayMiddlewaresFixrute fix;

  auto doCallBefore = [&]() {
    fix.rate_limit_middleware.before_handle(fix.req, fix.res, fix.rate_ctx, fix.dummy_parent_ctx);
  };

  auto doCallAfter = [&]() {
    fix.rate_limit_middleware.after_handle(fix.req, fix.res, fix.rate_ctx, fix.dummy_parent_ctx);
  };

  SECTION(
      "Section rate_limit not allowed expected return ratelimitexceed code "
      "and message") {
    doCallBefore();

    REQUIRE(fix.res.is_completed());
    REQUIRE(fix.res.code == StatusCodes::rateLimit);
    REQUIRE(fix.res.body == IssueMessages::rateLimitExceed);
  }

  SECTION("Section rate_limit is allowed expected return not completed task") {
    fix.rate_limiter.should_fail = false;

    doCallBefore();

    REQUIRE_FALSE(fix.res.is_completed());
  }

  SECTION("After handle expected no throw") { REQUIRE_NOTHROW(doCallAfter()); }
}

TEST_CASE("Test LogMiddleware") {
  TestGatewayMiddlewaresFixrute fix;

  auto doCallBefore = [&]() {
    fix.logging_middleware.before_handle(fix.req, fix.res, fix.log_ctx, fix.dummy_parent_ctx);
  };

  auto doCallAfter = [&]() {
    fix.logging_middleware.after_handle(fix.req, fix.res, fix.log_ctx, fix.dummy_parent_ctx);
  };

  SECTION("Before handle expected no throw") { REQUIRE_NOTHROW(doCallBefore()); }

  SECTION("After handle expected no throw") { REQUIRE_NOTHROW(doCallAfter()); }
}

TEST_CASE("Test cacheMiddleware") {
  TestGatewayMiddlewaresFixrute fix;
  fix.req.method = "GET"_method;

  auto doCallBefore = [&]() {
    fix.cache_middleware.before_handle(fix.req, fix.res, fix.cache_ctx, fix.dummy_parent_ctx);
  };

  auto doCallAfter = [&]() {
    fix.cache_middleware.after_handle(fix.req, fix.res, fix.cache_ctx, fix.dummy_parent_ctx);
  };

  SECTION("Method not GET expected res is not completed") {
    fix.req.method = "POST"_method;

    doCallBefore();

    REQUIRE_FALSE(fix.res.is_completed());
  }

  SECTION("Cache not returned expected res is not completed") {
    doCallBefore();
    REQUIRE_FALSE(fix.res.is_completed());
  }

  SECTION(
      "Cache returned expected res is completed and success status code "
      "and res.body == value from "
      "cache") {
    std::string value_from_cache = "cached_value_from_cache";
    fix.cache.mock_answer = value_from_cache;
    doCallBefore();
    REQUIRE(fix.res.is_completed());
    REQUIRE(fix.res.code == StatusCodes::success);
    REQUIRE(fix.res.body == value_from_cache);
  }

  SECTION("Cache returned expected metrics_ctx hit_cache = true") {
    std::string value_from_cache = "cached_value_from_cache";
    fix.cache.mock_answer = value_from_cache;
    doCallBefore();

    REQUIRE(fix.dummy_parent_ctx.metrics_ctx.hit_cache == true);
  }

  SECTION("After handle expected with no-get method expected not set") {
    fix.req.method = "POST"_method;
    int before_call_cache_set = fix.cache.call_set;

    doCallAfter();

    REQUIRE(fix.cache.call_set == before_call_cache_set);
  }

  SECTION("Get method expected set value in cache") {
    fix.req.method = "GET"_method;
    fix.req.url = "/test/auth";
    fix.req.body = "user=2";
    int before_call_cache_set = fix.cache.call_set;
    fix.res.body = "mock_set_cache";

    doCallAfter();

    REQUIRE(fix.cache.call_set == before_call_cache_set + 1);
    REQUIRE(fix.cache.last_set_key == "cache:GET:/test/auth|user=2|body=user=2");
    REQUIRE(fix.cache.last_set_value == fix.res.body);
  }
}

TEST_CASE("Test metrics middlewares") {
  TestGatewayMiddlewaresFixrute fix;

  auto doCallBefore = [&]() {
    fix.metrics_middleware.before_handle(fix.req, fix.res, fix.metrics_ctx, fix.dummy_parent_ctx);
  };

  auto doCallAfter = [&]() {
    MetricsMiddleware::context metrics_ctx;
    metrics_ctx.tracker.startTimer(&fix.metrics);
    fix.metrics_middleware.after_handle(fix.req, fix.res, metrics_ctx, fix.dummy_parent_ctx);
  };

  SECTION("Expected before_handle call new request") {
    int before_call_new_request = fix.metrics.call_newRequest;
    doCallBefore();
    REQUIRE(fix.metrics.call_newRequest == before_call_new_request + 1);
  }

  SECTION("Expected after_handle call request ended and save latency") {
    int before_call_request_end = fix.metrics.call_requestEnded;
    int before_call_save_latency = fix.metrics.call_saveRequestLatency;
    doCallAfter();
    CHECK(fix.metrics.call_requestEnded == before_call_request_end + 1);
    CHECK(fix.metrics.call_saveRequestLatency == before_call_save_latency + 1);
  }
}
