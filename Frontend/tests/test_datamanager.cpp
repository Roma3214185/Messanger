#include <catch2/catch_all.hpp>

#include "Debug_profiling.h"
#include "managers/datamanager.h"

struct TestDataManager : public DataManager {
  using DataManager::getNumberOfExistingUsers;
  using DataManager::getNumberOfMessageModels;
};

TEST_CASE("Test datamanager works with chats") {
  TestDataManager data_manager;
  auto private_chat1 = ChatFactory::createPrivateChat(1, "Ivan", "ivan228", 4, "offline");
  auto private_chat2 = ChatFactory::createPrivateChat(2, "Roma", "roma228", 5, "offline");
  auto private_chat3 = ChatFactory::createPrivateChat(3, "Sanya", "sanya228", 6, "offline");
  auto private_chat4 = ChatFactory::createPrivateChat(4, "Kolya", "kolya228", 7, "offline");
  setContractHandler(throwOnViolation);

  User valid_user;
  valid_user.id = 5;
  valid_user.name = "Roma";
  valid_user.tag = "RomaTag";
  valid_user.avatarPath = "roma/avatar";
  valid_user.email = "email_roma";

  SECTION("Add two same chats expected add last one") {
    auto same_private_chat = ChatFactory::createPrivateChat(1, "Roma", "roma228", 5, "offline");
    data_manager.addChat(private_chat1);
    data_manager.addChat(same_private_chat);

    REQUIRE(data_manager.getNumberOfExistingChats() == 1);
    REQUIRE(data_manager.getNumberOfMessageModels() == 1);
  }

  SECTION("Clear all chats works as expected") {
    data_manager.addChat(private_chat1);
    data_manager.addChat(private_chat2);
    data_manager.addChat(private_chat3);
    data_manager.addChat(private_chat4);
    REQUIRE(data_manager.getNumberOfExistingChats() == 4);

    data_manager.clearAllChats();

    REQUIRE(data_manager.getNumberOfExistingChats() == 0);
  }

  SECTION("Get existing private chat with user") {
    data_manager.addChat(private_chat1);
    data_manager.addChat(private_chat2);
    data_manager.addChat(private_chat3);
    data_manager.addChat(private_chat4);

    auto returned_chat = data_manager.getPrivateChatWithUser(6);

    REQUIRE(returned_chat != nullptr);
    REQUIRE(returned_chat->title == private_chat3->title);
  }

  SECTION("Get existing chat expected return same chat") {
    data_manager.addChat(private_chat1);

    auto chat = data_manager.getChat(private_chat1->chat_id);

    REQUIRE(chat != nullptr);
    REQUIRE(chat->isPrivate() == true);
    auto returned_chat = std::dynamic_pointer_cast<PrivateChat>(chat);
    REQUIRE(returned_chat != nullptr);
    REQUIRE(returned_chat->chat_id == 1);
    REQUIRE(returned_chat->user_id == 4);
    REQUIRE(returned_chat->title == "Ivan");
    REQUIRE(returned_chat->user_tag == "ivan228");
  }

  SECTION("There is no chats with give user_id expected return nullptr") {
    auto chat = data_manager.getPrivateChatWithUser(2);
    REQUIRE(chat == nullptr);
  }

  SECTION("Add chat with invalid id require throw exception") {
    auto group_chat = ChatFactory::createGroupChat(0, "Test title", 2, {}, {}, {});

    REQUIRE_THROWS(data_manager.addChat(group_chat));
  }

  SECTION("Get existing chat expected works as expected") {
    auto group_chat = ChatFactory::createGroupChat(1, "Test title", 2, {}, {}, {});
    data_manager.addChat(group_chat);
    REQUIRE(data_manager.getNumberOfExistingChats() == 1);

    auto returned_chat = data_manager.getChat(1);
    REQUIRE(returned_chat != nullptr);
  }

  SECTION("Get not existing chat expected return nullptr") {
    auto group_chat = ChatFactory::createGroupChat(1, "Test title", 2, {}, {}, {});
    data_manager.addChat(group_chat);
    REQUIRE(data_manager.getNumberOfExistingChats() == 1);

    auto returned_chat = data_manager.getChat(5);
    REQUIRE(returned_chat == nullptr);
  }

  SECTION("Get chatmodel for existing chat expected return not nullptr") {
    data_manager.addChat(private_chat1);
    REQUIRE(data_manager.getNumberOfExistingChats() == 1);

    auto returned_model = data_manager.getMessageModel(private_chat1->chat_id);
    REQUIRE(returned_model != nullptr);
  }

  SECTION("Get message model for not existing chat expected not nullptr") {
    REQUIRE(data_manager.getMessageModel(4) != nullptr);
  }

  SECTION("Clear all message models works as expected") {
    data_manager.addChat(private_chat1);
    data_manager.addChat(private_chat2);
    REQUIRE(data_manager.getNumberOfMessageModels() == 2);

    data_manager.clearAllMessageModels();

    REQUIRE(data_manager.getNumberOfMessageModels() == 0);
  }

  SECTION("Add valid user to empty storage expected number of users equals 1") {
    valid_user.id = 4;
    data_manager.saveUser(valid_user);

    REQUIRE(data_manager.getNumberOfExistingUsers() == 1);
  }

  SECTION("Add two users with same id expected add only last one") {
    int common_user_id = 4;
    valid_user.id = common_user_id;

    User valid_user2;
    valid_user2.id = common_user_id;
    valid_user2.email = "email_ivan";
    valid_user2.name = "Ivan";
    valid_user2.tag = "IvanTag";
    valid_user2.avatarPath = "user/avatar";

    data_manager.saveUser(valid_user);
    data_manager.saveUser(valid_user2);

    REQUIRE(data_manager.getNumberOfExistingUsers() == 1);
    auto returned_user = data_manager.getUser(common_user_id);
    REQUIRE(returned_user != std::nullopt);
    auto user_name = returned_user->name;
    LOG_INFO("Returned name = {}", user_name.toStdString());
    REQUIRE(returned_user->name == "Ivan");
  }

  SECTION("Add user with invalid id expecter throw exception") {
    User invalid_user;
    invalid_user.id = 0;
    REQUIRE_THROWS(data_manager.saveUser(invalid_user));
  }

  SECTION("Clear method clear all data") {
    valid_user.id = 2;
    data_manager.addChat(private_chat1);
    data_manager.saveUser(valid_user);
    REQUIRE(data_manager.getNumberOfExistingChats() == 1);
    REQUIRE(data_manager.getNumberOfMessageModels() == 1);
    REQUIRE(data_manager.getNumberOfExistingUsers() == 1);

    data_manager.clearAll();

    REQUIRE(data_manager.getNumberOfExistingChats() == 0);
    REQUIRE(data_manager.getNumberOfMessageModels() == 0);
    REQUIRE(data_manager.getNumberOfExistingUsers() == 0);
  }
}
