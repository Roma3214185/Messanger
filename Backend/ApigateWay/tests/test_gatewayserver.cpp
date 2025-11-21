#include <catch2/catch_all.hpp>

#include "gatewayserver.h"
#include "mocks/MockCache.h"
#include "mocks/MockProxy.h"
#include "mocks/MockConfigProvider.h"

TEST_CASE("First test") {
  crow::SimpleApp app;
  MockCache cache;
  MockProxy proxy;
  MockConfigProvider provider;

  GatewayServer server(app, &cache, &proxy, &provider);
}
