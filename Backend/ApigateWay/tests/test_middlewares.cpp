#include <catch2/catch_all.hpp>

#include <crow.h>

#include "middlewares/AuthMiddleware.h"
#include "middlewares/CacheMiddleware.h"
#include "middlewares/LoggingMiddleware.h"
#include "middlewares/RateLimitMiddleware.h"
#include "mocks/MockUtils.h"
#include "mocks/MockApiCache.h"
#include "mocks/MockVerifier.h"
#include "mocks/MockRateLimiter.h"
#include "mocks/MockTheadPool.h"
#include "mocks/MockMetrics.h"
#include "mocks/MockConfigProvider.h"

struct DummyParentCtx {};

struct TestGatewayMiddlewaresFixrute {
    LoggingMiddleware logging_middleware;
    RateLimitMiddleware rate_limit_middleware;
    AuthMiddleware auth_middleware;
    CacheMiddleware cache_middleware;
    MockApiCache cache;
    MockConfigProvider provider;
    crow::request req;
    crow::response res;
    MockMetrics metrics;
    MockVerifier verifier;
    MockRateLimiter rate_limiter;
    DummyParentCtx parent_ctx;
    MockThreadPool pool;
    int user_id = 123;

    TestGatewayMiddlewaresFixrute()
        : provider(MockUtils::getMockCodes()) {
      auth_middleware.verifier_ = &verifier;
      auth_middleware.provider = &provider;
      cache_middleware.cache_ = &cache;
      rate_limit_middleware.rate_limiter_ = &rate_limiter;
      rate_limit_middleware.provider_ = &provider;

      rate_limiter.should_fail = true;
      verifier.mock_ans = std::nullopt;
    }
};


TEST_CASE("Test Authmiddleware") {
  TestGatewayMiddlewaresFixrute fix;
  fix.req.url = "/auth/me";  // url that needs verification

  auto doCallBefore = [&]() {
    AuthMiddleware::context ctx;
    fix.auth_middleware.before_handle(fix.req, fix.res, ctx, fix.parent_ctx);
  };

  auto doCallAfter = [&]() {
    AuthMiddleware::context ctx;
    fix.auth_middleware.after_handle(fix.req, fix.res, ctx, fix.parent_ctx);
  };

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

  SECTION("Token not verifies expected unauthorized code and message about invalid token") {
    doCallBefore();
    REQUIRE(fix.res.is_completed());
    REQUIRE(fix.res.code == fix.provider.statusCodes().unauthorized);
    REQUIRE(fix.res.body == fix.provider.issueMessages().invalidToken);
  }

  SECTION("After handle expected no throw") {
    REQUIRE_NOTHROW(doCallAfter());
  }
}
