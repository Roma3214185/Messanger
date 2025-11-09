#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>

#include "../include/DataInputService.h"

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
    auto    r = tagValidDetailed(tag, cfg);
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

TEST_CASE("Email validation - aliases, subdomains, IP literals, quoted local, boundaries",
          "[email]") {
  Config cfg;
  cfg.kMinEmailLocalPartLength = 1;
  cfg.kMaxEmailLocalPartLength = 64;

  SECTION("Simple valid") {
    auto r = emailValidDetailed("user@gmail.com", cfg);
    REQUIRE(r.valid);
  }

  SECTION("Plus alias") {
    auto r = emailValidDetailed("user+alias@gmail.com", cfg);
    REQUIRE(r.valid);
  }

  SECTION("Quoted local") {
    auto r = emailValidDetailed("\"john..doe\"@gmail.com", cfg);
    REQUIRE(r.valid);
  }

  SECTION("Local part too long") {
    QString local(cfg.kMaxEmailLocalPartLength + 1, 'a');
    auto    r = emailValidDetailed(local + "@gmail.com", cfg);
    REQUIRE(!r.valid);
    REQUIRE(r.message == "Local part too long");
  }

  SECTION("Local part too short") {
    auto r = emailValidDetailed("@gmail.com", cfg);
    REQUIRE(!r.valid);
    REQUIRE(r.message == "Local part is empty");
  }

  SECTION("Missing at") {
    auto r = emailValidDetailed("notanemail", cfg);
    REQUIRE(!r.valid);
    REQUIRE(r.message == "Email does not contain @");
  }

  SECTION("Empty domain") {
    auto r = emailValidDetailed("user@", cfg);
    REQUIRE(!r.valid);
    REQUIRE(r.message == "Domain is empty");
  }

  SECTION("Invalid email") {
    auto r = emailValidDetailed("user@gmaill.com", cfg);
    REQUIRE(!r.valid);
    REQUIRE(r.message == "Invalid domain");
  }
}

TEST_CASE("Chained validation - form level", "[form]") {
  Config cfg;
  SECTION("Good request expected result is valid") {
    SignUpRequest good_request{.name     = "John Doe",
                               .email    = "john+dev@gmail.com",
                               .password = "GoodP@ss1",
                               .tag      = "dev_tag"};
    auto          r = validateRegistrationUserInput(good_request, cfg);
    LOG_WARN("Message: {}", r.message.toStdString());
    REQUIRE(r.valid);
  }

  SECTION("Request with invalid name expected valid error message") {
    SignUpRequest bad_name_request{
        .name = "", .email = "john+dev@example.com", .password = "GoodP@ss1", .tag = "dev_tag"};
    auto r2 = validateRegistrationUserInput(bad_name_request, cfg);
    REQUIRE(!r2.valid);
    LOG_WARN("Message: {}", r2.message.toStdString());
    REQUIRE(r2.message == "Name is empty");
  }

  SECTION("Request with invalid email expected valid error message") {
    SignUpRequest bad_email_request{
        .name = "John Doe", .email = "invalidemail", .password = "GoodP@ss1", .tag = "dev_tag"};
    auto r3 = validateRegistrationUserInput(bad_email_request, cfg);
    REQUIRE(!r3.valid);
    LOG_WARN("Message: {}", r3.message.toStdString());
    REQUIRE(r3.message == "Email does not contain @");
  }

  SECTION("Request with invalid password expected valid error message") {
    SignUpRequest bad_password_request{
        .name = "John Doe", .email = "john@gmail.com", .password = "short", .tag = "dev_tag"};
    auto r4 = validateRegistrationUserInput(bad_password_request, cfg);
    REQUIRE(!r4.valid);
    LOG_WARN("Message: {}", r4.message.toStdString());
    REQUIRE(r4.message == "Password is too short");
  }

  SECTION("Request with invalid tag expected valid error message") {
    SignUpRequest bad_tag_request{
        .name = "John Doe", .email = "john@gmail.com", .password = "GoodP@ss1", .tag = "_tag"};
    auto r5 = validateRegistrationUserInput(bad_tag_request, cfg);
    REQUIRE(!r5.valid);
    LOG_WARN("Message from tag: {}", r5.message.toStdString());
    REQUIRE(r5.message == "First character must be letter or number");
  }
}

TEST_CASE("Parameterized invalid passwords", "[password][param]") {
  Config cfg;
  cfg.kMinPasswordLength = 6;
  cfg.kMaxPasswordLength = 12;

  auto password_sample = GENERATE("short", "has space", "bad|char", "\nnewline");
  auto r               = passwordValidDetailed(password_sample, cfg);
  REQUIRE(!r.valid);
}
