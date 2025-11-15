#include <catch2/catch_all.hpp>

#include "messageservice/managers/MessageManager.h"
#include "mocks/FakeSqlExecutor.h"
#include "mocks/MockDatabase.h"
#include "mocks/MockCache.h"
#include "mocks/MockTheadPool.h"

TEST_CASE("Test") {
  MockDatabase db;
  MockThreadPool pool;
  MockCache cache;
  FakeSqlExecutor executor;
  GenericRepository repository(db, &executor, cache, &pool);
  MessageManager manager(&repository, &executor);

  SECTION("getChatMessages expected create valid sql request") {
    //GetChatMessa
    //manager.getChatMessages(pack);
  }


}
