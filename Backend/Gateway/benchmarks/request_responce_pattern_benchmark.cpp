#include <crow/crow.h>
#include <benchmark/benchmark.h>

#include "gatewayserver.h"
#include "mocks/MockClient.h"
#include "mocks/MockConfigProvider.h"
#include "mocks/MockUtils.h"
#include "mocks/MockApiCache.h"
#include "mocks/MockVerifier.h"
#include "mocks/MockRateLimiter.h"
#include "mocks/MockTheadPool.h"
#include "mocks/MockMetrics.h"

struct BenchmarkGatewayServerFixrute {
    GatewayApp app;
    MockApiCache cache;
    MockClient client;
    MockConfigProvider provider;
    crow::request req;
    crow::response res;
    GatewayServer server;
    MockMetrics metrics;
    std::string mock_client_ans = "TEST FORWARD";
    int mock_client_code = 1356;
    MockVerifier verifier;
    MockRateLimiter rate_limiter;
    MockThreadPool pool;
    int user_id = 123;

    BenchmarkGatewayServerFixrute()
        : provider(MockUtils::getMockPorts())
        , server(app, &client, &pool, &provider) {
      app.get_middleware<AuthMiddleware>().verifier_ = &verifier;
      app.get_middleware<CacheMiddleware>().cache_ = &cache;
      app.get_middleware<LoggingMiddleware>();
      app.get_middleware<RateLimitMiddleware>().rate_limiter_ = &rate_limiter;
      app.get_middleware<MetricsMiddleware>().metrics_ = &metrics;

      provider.mock_codes = MockUtils::getMockCodes();
      client.wait_for = std::chrono::milliseconds(150);

      verifier.mock_ans = user_id;
      client.mock_response = std::make_pair(mock_client_code, mock_client_ans);

      server.registerRoutes();
      req.method = crow::HTTPMethod::Post;
      req.url = "/auth/register";
      app.validate();
    }

    void makeCall() {
      app.validate();
      app.handle_full(req, res);
    }
};

static void BM_SyncRequestResponce(benchmark::State& state) {
  BenchmarkGatewayServerFixrute fix;

  for (auto _ : state) {
    for (int i = 0; i < state.range(0); i++) {
      fix.makeCall();
      benchmark::DoNotOptimize(fix);
    }
  }
}

BENCHMARK(BM_SyncRequestResponce)->Arg(10)->Arg(100)->Arg(1000);
