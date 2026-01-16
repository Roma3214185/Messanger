#include <QByteArray>
#include <QJsonArray>
#include <QJsonObject>
#include <QSignalSpy>
#include <QTest>
#include <QUrl>
#include <QWaitCondition>
#include <catch2/catch_all.hpp>

#include "managers/usermanager.h"
#include "mocks/MockAccessManager.h"

class TestUserManager : public UserManager {
 public:
  using UserManager::onFindUsersByTag;
  using UserManager::onGetUser;
  using UserManager::UserManager;
};

TEST_CASE("Test user manager") {
  MockReply mock_reply;
  MockNetworkAccessManager network_manager(&mock_reply);
  QUrl url("http://localhost:8083/");
  std::chrono::milliseconds times_out{20};
  std::chrono::milliseconds delay{3};
  TokenManager token_manager;
  long long test_current_id = 12345;
  QString token = "secret-token123";
  token_manager.setData(token, test_current_id);
  EntityFactory entity_factory(&token_manager);
  UserManager user_manager(&network_manager, url, &entity_factory, times_out);
  int user_id{4};
  auto reply = std::make_unique<MockReply>();
  network_manager.setReply(reply.get());
  User user{.id = 1, .name = "roma", .email = "roma@gmail.com", .tag = "roma229", .avatarPath = "path/to/avatar"};

  QJsonObject obj{
      {"id", user.id}, {"name", user.name}, {"email", user.email}, {"tag", user.tag}, {"avatar_path", user.avatarPath}};
  QJsonDocument doc(obj);
  QByteArray json_data = doc.toJson();
  auto doGetUser = [&]() -> QFuture<std::optional<User>> { return user_manager.getUser(user_id, token); };

  SECTION("GetUserExpectedRightUrlCreated") {
    QUrl resolved_url("http://localhost:8083/users/4");
    doGetUser();
    REQUIRE(network_manager.last_request.url() == resolved_url);
  }

  SECTION("GetUserExpectedRightSetToken") {
    QUrl resolved_url("http://localhost:8083/users/4");
    doGetUser();
    REQUIRE(network_manager.last_request.rawHeader("Authorization") == token);
  }

  SECTION("GetUserExpectedGetMethod") {
    int before_get_calls = network_manager.get_counter;
    doGetUser();
    REQUIRE(network_manager.get_counter == before_get_calls + 1);
  }

  SECTION("ErrorReplyExpectedEmittedErrorOccuredWithValidText") {
    QSignalSpy spyErrorOccured(&user_manager, &BaseManager::errorOccurred);
    int before_calls = spyErrorOccured.count();
    auto mock_reply = std::make_unique<MockReply>();
    mock_reply->setMockError(QNetworkReply::AuthenticationRequiredError, "error");
    network_manager.setReply(mock_reply.get());

    network_manager.shouldFail = true;
    doGetUser();
    QCoreApplication::processEvents();

    REQUIRE(spyErrorOccured.count() == before_calls + 1);
    auto arguments = spyErrorOccured.takeFirst();
    REQUIRE(arguments.at(0).toString().toStdString() == "Error occurred: error");
  }

  SECTION("ErrorReplyExpectedReturnedNullopt") {
    auto mock_reply = std::make_unique<MockReply>();
    mock_reply->setMockError(QNetworkReply::AuthenticationRequiredError, "error");
    network_manager.setReply(mock_reply.get());

    // QTimer::singleShot(0, mock_reply, &MockReply::emitFinished);
    auto future = doGetUser();
    QCoreApplication::processEvents();

    std::optional<User> user = future.result();
    REQUIRE(user == std::nullopt);
  }

  SECTION("ValidRespondFromServerExpectedNoErrorOccurred") {
    QSignalSpy spyErrorOccured(&user_manager, &UserManager::errorOccurred);
    int before_calls = spyErrorOccured.count();
    auto mock_reply = std::make_unique<MockReply>();
    mock_reply->setData(json_data);
    network_manager.setReply(mock_reply.get());

    // QTimer::singleShot(0, mock_reply, &MockReply::emitFinished);
    doGetUser();
    QCoreApplication::processEvents();

    REQUIRE(spyErrorOccured.count() == before_calls);
  }

  SECTION("ValidRespondFromServerExpectedValidUserReturned") {
    auto mock_reply = std::make_unique<MockReply>();
    mock_reply->setData(json_data);
    network_manager.setReply(mock_reply.get());

    // QTimer::singleShot(0, mock_reply, &MockReply::emitFinished);
    auto future = doGetUser();
    QCoreApplication::processEvents();

    std::optional<User> user_from_server = future.result();

    REQUIRE(user_from_server != std::nullopt);
    REQUIRE(user_from_server->name == user.name);
    REQUIRE(user_from_server->email == user.email);
    REQUIRE(user_from_server->tag == user.tag);
    REQUIRE(user_from_server->id == user.id);
    REQUIRE(user_from_server->avatarPath == user.avatarPath);
  }
}

TEST_CASE("Test onGetUser") {
  User user{.id = 1, .name = "roma", .email = "roma@gmail.com", .tag = "roma229", .avatarPath = "path/to/avatar"};

  QJsonObject obj{
      {"id", user.id}, {"name", user.name}, {"email", user.email}, {"tag", user.tag}, {"avatar_path", user.avatarPath}};
  QJsonDocument doc(obj);
  QByteArray valid_json_data = doc.toJson();
  MockReply mock_reply;
  MockNetworkAccessManager network_manager(&mock_reply);
  QUrl url("url");
  TokenManager token_manager;
  long long test_current_id = 12345;
  QString token = "secret-token123";
  token_manager.setData(token, test_current_id);
  EntityFactory entity_factory(&token_manager);
  TestUserManager user_manager(&network_manager, url, &entity_factory);

  SECTION("ValidReplyExpectedNotEmittedErrorOccurred") {
    QSignalSpy spyErrorOccured(&user_manager, &UserManager::errorOccurred);
    int before_calls = spyErrorOccured.count();
    auto mock_reply = std::make_unique<MockReply>();
    mock_reply->setData(valid_json_data);
    network_manager.setReply(mock_reply.get());

    user_manager.onGetUser(mock_reply->readAll());
    QCoreApplication::processEvents();

    REQUIRE(spyErrorOccured.count() == before_calls);
  }

  SECTION("ValidReplyExpectedReturnedValidResult") {
    auto mock_reply = std::make_unique<MockReply>();
    mock_reply->setData(valid_json_data);
    network_manager.setReply(mock_reply.get());

    auto result = user_manager.onGetUser(mock_reply->readAll());
    QCoreApplication::processEvents();

    REQUIRE(result.has_value());
    REQUIRE(result->name == user.name);
    REQUIRE(result->email == user.email);
    REQUIRE(result->tag == user.tag);
    REQUIRE(result->avatarPath == user.avatarPath);
    REQUIRE(result->id == user.id);
  }

  SECTION("InvalidJsonExpectedReturnNulloptWithErrorSignal") {
    QSignalSpy spyErrorOccured(&user_manager, &UserManager::errorOccurred);
    int before_calls = spyErrorOccured.count();

    QByteArray invalid_json_data = "this is not json";
    auto mock_reply = std::make_unique<MockReply>();
    mock_reply->setData(invalid_json_data);
    mock_reply->setMockError(QNetworkReply::NoError, "");
    network_manager.setReply(mock_reply.get());

    auto result = user_manager.onGetUser(mock_reply->readAll());
    QCoreApplication::processEvents();

    REQUIRE_FALSE(result.has_value());
    REQUIRE(spyErrorOccured.count() == before_calls + 1);
  }

  SECTION("JsonArrayInsteadOfObjectExpectedReturnNulloptWithErrorSignal") {
    QSignalSpy spyErrorOccured(&user_manager, &UserManager::errorOccurred);
    int before_calls = spyErrorOccured.count();

    QJsonArray jsonArray{1, 2, 3};
    QJsonDocument doc(jsonArray);
    QByteArray invalid_json_data = doc.toJson();

    auto mock_reply = std::make_unique<MockReply>();
    mock_reply->setData(invalid_json_data);
    mock_reply->setMockError(QNetworkReply::NoError, "");
    network_manager.setReply(mock_reply.get());

    auto result = user_manager.onGetUser(mock_reply->readAll());
    QCoreApplication::processEvents();

    REQUIRE_FALSE(result.has_value());
    REQUIRE(spyErrorOccured.count() == before_calls + 1);
  }

  // SECTION("EmptyJsonObjectExpectedReturnNulloptNoErrorSignal") {
  //   QSignalSpy spyErrorOccured(&user_manager, &UserManager::errorOccurred);
  //   int before_calls = spyErrorOccured.count();

  //   QJsonObject emptyObj;
  //   QJsonDocument emptyDoc(emptyObj);
  //   QByteArray empty_json_data = emptyDoc.toJson();

  //   auto mock_reply = std::make_unique<MockReply>();
  //   mock_reply->setData(empty_json_data);
  //   mock_reply->setMockError(QNetworkReply::NoError, "");
  //   network_manager.setReply(mock_reply.get());

  //   auto result = user_manager.onGetUser(mock_reply->readAll());
  //   QCoreApplication::processEvents();

  //   REQUIRE(result.has_value());
  //   REQUIRE(spyErrorOccured.count() == before_calls);
  // }
}

TEST_CASE("Test findUsersByTag") {
  MockReply mock_reply;
  MockNetworkAccessManager network_manager(&mock_reply);
  QUrl url("http://localhost:8083/");
  std::chrono::milliseconds times_out{20};
  std::chrono::milliseconds delay{5};
  TokenManager token_manager;
  long long test_current_id = 12345;
  QString token = "secret-token123";
  token_manager.setData(token, test_current_id);
  EntityFactory entity_factory(&token_manager);
  UserManager user_manager(&network_manager, url, &entity_factory, times_out);
  QString tag = "roma222";
  auto reply = std::make_unique<MockReply>();
  network_manager.setReply(reply.get());

  QList<User> users;
  User user{.id = 1, .name = "roma", .email = "roma@gmail.com", .tag = "roma229", .avatarPath = "path/to/avatar"};

  users.push_back(user);
  users.push_back(user);
  users.push_back(user);
  users.push_back(user);
  users.push_back(user);

  QJsonArray array;
  for (auto user : users) {
    QJsonObject obj{{"id", user.id},
                    {"name", user.name},
                    {"email", user.email},
                    {"tag", user.tag},
                    {"avatar_path", user.avatarPath}};
    array.append(obj);
  }

  QJsonObject root;
  root["users"] = array;

  QJsonDocument doc(root);
  QByteArray json_data = doc.toJson();
  QString currect_token = "test-token-123";

  SECTION("ExpectedFucntionCreateRightUrl") {
    user_manager.findUsersByTag(tag, currect_token);

    REQUIRE(network_manager.last_request.url() == QUrl("http://localhost:8083/users/search?tag=roma222"));
  }

  SECTION("ExpectedGetRequestSended") {
    int before_calls = network_manager.get_counter;
    user_manager.findUsersByTag(tag, currect_token);

    REQUIRE(network_manager.get_counter == before_calls + 1);
  }

  SECTION("NoRespondFromServerExpectedErrorOccurred") {
    QSignalSpy spyErrorOccurred(&user_manager, &UserManager::errorOccurred);
    int before_calls = spyErrorOccurred.count();
    network_manager.shouldReturnResponce = false;
    auto future = user_manager.findUsersByTag(tag, currect_token);
    std::string expected_responce = "Server didn't respond";

    QTRY_COMPARE_WITH_TIMEOUT(spyErrorOccurred.count(), before_calls + 1, times_out + delay);

    auto arguments = spyErrorOccurred.takeFirst();
    REQUIRE(arguments.at(0).toString().toStdString() == expected_responce);
  }

  SECTION("NoRespondFromServerExpectedReturnedEmptyList") {
    QSignalSpy spyErrorOccurred(&user_manager, &UserManager::errorOccurred);
    int before_calls = spyErrorOccurred.count();
    network_manager.shouldReturnResponce = false;
    auto future = user_manager.findUsersByTag(tag, currect_token);

    QTRY_COMPARE_WITH_TIMEOUT(spyErrorOccurred.count(), before_calls + 1, times_out + delay);
    REQUIRE(future.result().isEmpty());
  }

  SECTION("ReturnedErrorFromServerEmpectedEmittedError") {
    auto reply_with_error = std::make_unique<MockReply>();
    reply_with_error->setMockError(QNetworkReply::AuthenticationRequiredError, "there is no authentification");
    network_manager.setReply(reply_with_error.get());
    QSignalSpy spyErrorOccurred(&user_manager, &BaseManager::errorOccurred);
    int before_calls = spyErrorOccurred.count();
    network_manager.shouldFail = true;

    // QTimer::singleShot(0, reply_with_error, &MockReply::emitFinished);
    auto future = user_manager.findUsersByTag(tag, currect_token);
    QCoreApplication::processEvents();

    REQUIRE(spyErrorOccurred.count() == before_calls + 1);
    auto arguments = spyErrorOccurred.takeFirst();
    REQUIRE(arguments.at(0).toString().toStdString() == "Error occurred: there is no authentification");
  }

  SECTION("ValidReplyExpectedNotEmittedError") {
    auto reply = std::make_unique<MockReply>();
    reply->setData(json_data);
    network_manager.setReply(reply.get());
    QSignalSpy spyErrorOccurred(&user_manager, &UserManager::errorOccurred);
    int before_calls = spyErrorOccurred.count();

    QTimer::singleShot(0, reply.get(), &MockReply::emitFinished);
    auto future = user_manager.findUsersByTag(tag, currect_token);
    QCoreApplication::processEvents();

    REQUIRE(spyErrorOccurred.count() == before_calls);
  }
}

TEST_CASE("Tests onUserFindedByTag") {
  MockReply mock_reply;
  MockNetworkAccessManager network_manager(&mock_reply);
  QUrl url("http://localhost:8083/");
  std::chrono::milliseconds times_out{20};
  TokenManager token_manager;
  long long test_current_id = 12345;
  QString token = "secret-token123";
  token_manager.setData(token, test_current_id);
  EntityFactory entity_factory(&token_manager);
  TestUserManager user_manager(&network_manager, url, &entity_factory, times_out);
  QString tag = "roma222";
  auto reply = std::make_unique<MockReply>();
  network_manager.setReply(reply.get());

  QList<User> users;
  User user{.id = 1, .name = "roma", .email = "roma@gmail.com", .tag = "roma229", .avatarPath = "path/to/avatar"};

  users.push_back(user);
  users.push_back(user);
  users.push_back(user);
  users.push_back(user);
  users.push_back(user);

  QJsonArray array;
  for (auto user : users) {
    QJsonObject obj{{"id", user.id},
                    {"name", user.name},
                    {"email", user.email},
                    {"tag", user.tag},
                    {"avatar_path", user.avatarPath}};
    array.append(obj);
  }

  QJsonObject root;
  root["users"] = array;

  QJsonDocument doc(root);
  QByteArray valid_json_data = doc.toJson();

  SECTION("GivenValidReplyExpectedNoErrorOccurred") {
    auto reply = std::make_unique<MockReply>();
    reply->setData(valid_json_data);
    QSignalSpy spyErrorOccurred(&user_manager, &UserManager::errorOccurred);
    int before_calls = spyErrorOccurred.count();

    user_manager.onFindUsersByTag(reply->readAll());

    REQUIRE(spyErrorOccurred.count() == before_calls);
  }

  SECTION("GivenInvalidJsonExpectedErrorOccurred") {
    QJsonObject emptyObj;
    QJsonDocument emptyDoc(emptyObj);
    QByteArray empty_json_data = emptyDoc.toJson();

    QSignalSpy spyErrorOccurred(&user_manager, &UserManager::errorOccurred);
    int before_calls = spyErrorOccurred.count();

    user_manager.onFindUsersByTag(empty_json_data);

    REQUIRE(spyErrorOccurred.count() == before_calls + 1);
  }
}

TEST_CASE("UserManager onFindUsersByTag invalid JSON handling") {
  MockReply mock_reply;
  MockNetworkAccessManager network_manager(&mock_reply);
  QUrl url("http://localhost:8083/");
  std::chrono::milliseconds times_out{20};
  TokenManager token_manager;
  long long test_current_id = 12345;
  QString token = "secret-token123";
  token_manager.setData(token, test_current_id);
  EntityFactory entity_factory(&token_manager);
  TestUserManager user_manager(&network_manager, url, &entity_factory, times_out);
  QString tag = "roma222";
  auto reply = std::make_unique<MockReply>();
  network_manager.setReply(reply.get());

  SECTION("Invalid JSON: not a JSON at all") {
    QSignalSpy spyErrorOccured(&user_manager, &UserManager::errorOccurred);
    auto mock_reply = std::make_unique<MockReply>();
    mock_reply->setData("this is not json");

    auto users = user_manager.onFindUsersByTag(mock_reply->readAll());
    REQUIRE(users.empty());

    REQUIRE(spyErrorOccured.count() == 1);
    auto arguments = spyErrorOccured.takeFirst();
    REQUIRE(arguments.at(0).toString() == "Invalid JSON: expected object at root");
  }

  SECTION("JSON root is an array instead of object") {
    QSignalSpy spyErrorOccured(&user_manager, &UserManager::errorOccurred);
    auto mock_reply = std::make_unique<MockReply>();
    mock_reply->setData("[{\"id\":1}]");

    auto users = user_manager.onFindUsersByTag(mock_reply->readAll());
    REQUIRE(users.empty());

    REQUIRE(spyErrorOccured.count() == 1);
    auto arguments = spyErrorOccured.takeFirst();
    REQUIRE(arguments.at(0).toString() == "Invalid JSON: expected object at root");
  }

  SECTION("JSON object missing 'users' key expected error occurred") {
    QSignalSpy spyErrorOccured(&user_manager, &UserManager::errorOccurred);
    auto mock_reply = std::make_unique<MockReply>();
    mock_reply->setData(R"({"wrong_key": []})");

    auto users = user_manager.onFindUsersByTag(mock_reply->readAll());
    REQUIRE(users.empty());

    REQUIRE(spyErrorOccured.count() == 1);
  }

  SECTION("'users' key is not an array") {
    QSignalSpy spyErrorOccured(&user_manager, &UserManager::errorOccurred);
    auto mock_reply = std::make_unique<MockReply>();
    mock_reply->setData(R"({"users": 123})");

    auto users = user_manager.onFindUsersByTag(mock_reply.get()->readAll());
    REQUIRE(users.empty());

    REQUIRE(spyErrorOccured.count() == 0);
  }
}
