// #define CATCH_CONFIG_MAIN
// #include <catch2/catch_all.hpp>

// #include "DataInputService/datainputservice.h"

// namespace DISC = DataInputService::detail;

// TEST_CASE("Testing password checking in datainputservice", "[auth][password]") {
//   SECTION("ValidPasswordExpectedTrue") {
//     QString password = "12345678";
//     REQUIRE(DataInputService::passwordValid(password) == true);
//   }

//   SECTION("ValidPasswordExpectedTrue") {
//     QString password = "123455760";
//     REQUIRE(DataInputService::passwordValid(password) == true);
//   }

//   SECTION("PasswordWithInvalidCharacterExpectedFalse") {
//     QString password = "12345&6789";
//     REQUIRE(DataInputService::passwordValid(password) == false);
//   }

//   SECTION("PasswordWithTwoInvalidCharacterExpectedFalse") {
//     QString password = "12#345&6789";
//     REQUIRE(DataInputService::passwordValid(password) == false);
//   }

//   SECTION("PasswordWithEmptyCharacterExpectedFalse") {
//     QString password = "12345 36789";
//     REQUIRE(DataInputService::passwordValid(password) == false);
//   }

//   SECTION("PasswordWithEmpyCharacterInfrontExpectedFalse") {
//     QString password = " 1234536789";
//     REQUIRE(DataInputService::passwordValid(password) == false);
//   }

//   SECTION("PasswordWithEmpyCharactersExpectedFalse") {
//     QString password = " 12345    367893";
//     REQUIRE(DataInputService::passwordValid(password) == false);
//   }

//   SECTION("PasswordWithNotPermittedCharacterExpectedFalse") {
//     QString password = "12345&36789";
//     REQUIRE(DataInputService::passwordValid(password) == false);
//   }

//   SECTION("PasswordMoreThanMaxValidLengthCharactersExpectedFalse") {
//     const int maxValidLength = DISC::kMaxPasswordLength;
//     QString password = QString(maxValidLength + 2, 'b');

//     REQUIRE(DataInputService::passwordValid(password) == false);
//   }
// }

// TEST_CASE("Testing tag in datainputservice", "[auth][tag]") {
//   SECTION("TagEmptyExpectedFalse") {
//     QString tag = "";
//     REQUIRE(DataInputService::tagValid(tag) == false);
//   }

//   SECTION("TagLessThanMinValidTagLenExpectedFalse") {
//     const int minValidTagLen = DISC::kMinTagLength;
//     QString tag = QString(minValidTagLen - 1, 'a');
//     REQUIRE(DataInputService::tagValid(tag) == false);
//   }

//   SECTION("TagStartsWithUnderlineExpectedFalse") {
//     QString tag = "_r4241";
//     REQUIRE(DataInputService::tagValid(tag) == false);
//   }

//   SECTION("TagWithTwoUnderlineInARowExpectedFalse") {
//     QString tag = "r__r14";
//     REQUIRE(DataInputService::tagValid(tag) == false);
//   }

//   SECTION("TagWithDotExpectedFalse") {
//     QString tag = "r.r13";
//     REQUIRE(DataInputService::tagValid(tag) == false);
//   }
// }

// TEST_CASE("Testing email in datainputservice", "[auth][email]") {
//   SECTION("EmailWithoutDomeinExpectedFalse") {
//     QString email = "123456789";
//     REQUIRE(DataInputService::emailValid(email) == false);
//   }

//   SECTION("ValidEmailExpectedTrue") {
//     QString email = "roma@gmail.com";
//     REQUIRE(DataInputService::emailValid(email) == true);
//   }

//   SECTION("EmailWithOnlyDomeinExpectedFalse") {
//     QString email = "@gmail.com";
//     REQUIRE(DataInputService::emailValid(email) == false);
//   }

//   SECTION("EmailWithInvalidCharacterExpectedFalse") {
//     QString email = "roma*@gmail.com";
//     REQUIRE(DataInputService::emailValid(email) == false);
//   }

//   SECTION("EmailWithInvalidDomeinExpectedFalse") {
//     QString email = "roma*@gmailcom";
//     REQUIRE(DataInputService::emailValid(email) == false);
//   }

//   SECTION("EmailIsLessThanMinValidLenExpectedFalse") {
//     const int minValidEmailLen = DISC::kMinEmailLocalPartLength;
//     QString email = QString(minValidEmailLen - 1, 'a') + DISC::kEmailDomain;

//     REQUIRE(DataInputService::emailValid(email) == false);
//   }

//   SECTION("EmailIsMoreThanMaxValidLenExpectedFalse") {
//     const int maxValidEmailLen = DISC::kMaxEmailLocalPartLength;
//     QString email = QString(maxValidEmailLen + 1, 'a') + DISC::kEmailDomain;

//     REQUIRE(DataInputService::emailValid(email) == false);
//   }

//   SECTION("EmailWithEmptyCharaverExpectedFalse") {
//     QString email = "rom a@gmailcom";
//     REQUIRE(DataInputService::emailValid(email) == false);
//   }
//   SECTION("EmailWithEmptyCharaverInfrontExpectedFalse") {
//     QString email = "     roma@gmailcom";
//     REQUIRE(DataInputService::emailValid(email) == false);
//   }
// }

// TEST_CASE("Testing name in datainputservice", "[auth][name]") {
//   SECTION("EmptyNameExpectedFalse") {
//     QString name = "";
//     REQUIRE(DataInputService::nameValid(name) == false);
//   }

//   SECTION("NameMoreThanMaxValidNameLengthExpectedFalse") {
//     const int maxValidNameLength = DISC::kMaxLenOfName;
//     QString name = QString(maxValidNameLength + 1, 'a');

//     REQUIRE(DataInputService::nameValid(name) == false);
//   }

//   SECTION("NameLessThanMinValidNameLengthExpectedFalse") {
//     const int minValidNameLength = DISC::kMinLenOfName;
//     QString name = QString(minValidNameLength - 1, 'a');

//     REQUIRE(DataInputService::nameValid(name) == false);
//   }
// }
