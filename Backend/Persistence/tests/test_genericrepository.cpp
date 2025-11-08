#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>

#include "Persistence/GenericRepository.h"
#include "mocks/MockDatabase.h"

TEST_CASE("Test saving entity in database") {
  MockDatabase db;
  GenericRepository rep(db);

  rep.save("")

}
