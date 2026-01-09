#include <catch2/catch_all.hpp>

#include "models/UserModel.h"

TEST_CASE("Test user model") {
  UserModel user_model;
  User user;
  user.id = 1;
  user.email = "romanlobach@gmail.com";
  user.name = "Roma";
  user.tag = "romanlobach";
  user.avatarPath = "path/to/avatar";

  SECTION("Add user expected list of users increase by 1") {
    int before = user_model.rowCount();
    user_model.addUser(user);
    REQUIRE(user_model.rowCount() == before + 1);
  }

  SECTION("Add user two times expected list of users increase by 1") {
    int before = user_model.rowCount();
    user_model.addUser(user);
    user_model.addUser(user);
    REQUIRE(user_model.rowCount() == before + 1);
  }

  SECTION("Add user with invalid id expected throw exception") {
    user.id = 0;
    REQUIRE_THROWS(user_model.addUser(user));
  }

  SECTION("Add user expected return valid user data") {
    user_model.addUser(user);
    QModelIndex index = user_model.index(0, 0);
    REQUIRE(user_model.data(index, UserModel::AvatarRole).toString() == user.avatarPath);
    REQUIRE(user_model.data(index, UserModel::NameRole).toString() == user.name);
    REQUIRE(user_model.data(index, UserModel::TagRole).toString() == user.tag);
    REQUIRE(user_model.data(index, UserModel::EmailRole).toString() == user.email);
    REQUIRE(user_model.data(index, UserModel::UserIdRole).toInt() == user.id);
  }

  SECTION("Expected return valid hash map") {
    QHash<int, QByteArray> expected = {{UserModel::Roles::UserIdRole, "chat_id"},
                                       {UserModel::Roles::NameRole, "name"},
                                       {UserModel::Roles::TagRole, "tag"},
                                       {UserModel::Roles::EmailRole, "email"},
                                       {UserModel::Roles::AvatarRole, "avatar"}};

    REQUIRE(user_model.roleNames() == expected);
  }

  SECTION("Clear user models") {
    User user1{.id = 1};
    User user2{.id = 2};
    User user3{.id = 3};
    User user4{.id = 4};
    user_model.addUser(user1);
    user_model.addUser(user2);
    user_model.addUser(user3);
    user_model.addUser(user4);
    REQUIRE(user_model.rowCount() == 4);

    user_model.clear();

    REQUIRE(user_model.rowCount() == 0);
  }
}
