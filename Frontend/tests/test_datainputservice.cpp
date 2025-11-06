#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>

#include "DataInputService/datainputservice.h"

using namespace std;
using namespace DataInputService;

TEST_CASE("Name validation - UTF-8 friendly", "[name]") {
  Config cfg;
  cfg.kMinLenOfName = 1;
  cfg.kMaxLenOfName = 10;

  SECTION("Empty name") {
    auto r = nameValidDetailed("", cfg);
    REQUIRE(!r.valid);
    REQUIRE(r.message == "Name is empty");
  }

  SECTION("Exact min length") {
    auto r = nameValidDetailed("A", cfg);
    REQUIRE(r.valid);
  }

  SECTION("Exact max length") {
    auto r = nameValidDetailed("abcdefghij", cfg);
    REQUIRE(r.valid);
  }

  SECTION("Too short") {
    auto r = nameValidDetailed("", cfg);
    REQUIRE(!r.valid);
  }

  SECTION("Too long") {
    auto r = nameValidDetailed("abcdefghijk", cfg);
    REQUIRE(!r.valid);
  }

  SECTION("Control character") {
    auto r = nameValidDetailed("John\nDoe", cfg);
    REQUIRE(!r.valid);
    REQUIRE(r.message == "Name contains invalid character");
  }

  SECTION("Leading/trailing space") {
    auto r = nameValidDetailed(" John", cfg);
    REQUIRE(r.valid);
    auto r2 = nameValidDetailed("John ", cfg);
    REQUIRE(r2.valid);
  }

  SECTION("Invalid symbol") {
    auto r = nameValidDetailed("John@", cfg);
    REQUIRE(!r.valid);
    REQUIRE(r.message == "Name contains invalid character");
  }
}

