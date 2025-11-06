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

TEST_CASE("Tag validation - mixed patterns, boundaries, unicode", "[tag]") {
  Config cfg;
  cfg.kMinTagLength = 2;
  cfg.kMaxTagLength = 16;

  SECTION("Empty tag") {
    auto r = tagValidDetailed("", cfg);
    REQUIRE(!r.valid);
    REQUIRE(r.message == "Tag is empty");
  }

  SECTION("Starts with underscore") {
    auto r = tagValidDetailed("_abc", cfg);
    REQUIRE(!r.valid);
    REQUIRE(r.message == "First character must be letter or number");
  }

  SECTION("Consecutive underscores") {
    auto r = tagValidDetailed("ab__cd", cfg);
    REQUIRE(!r.valid);
    REQUIRE(r.message == "Tag contains consecutive underscores");
  }

  SECTION("Ends with underscore") {
    auto r = tagValidDetailed("tag_", cfg);
    REQUIRE(r.valid);
  }

  SECTION("Max length tag") {
    QString tag(cfg.kMaxTagLength, 'a');
    auto r = tagValidDetailed(tag, cfg);
    REQUIRE(r.valid);
  }

  SECTION("Unicode tag") {
    auto r = tagValidDetailed("тег1", cfg);
    REQUIRE(r.valid);
  }

  SECTION("Mixed valid/invalid pattern") {
    auto r = tagValidDetailed("tag_name-1.", cfg);
    REQUIRE(r.valid);
  }

  SECTION("Invalid symbol") {
    auto r = tagValidDetailed("tag!name", cfg);
    REQUIRE(!r.valid);
    REQUIRE(r.message == "Tag contains invalid character");
  }
}



