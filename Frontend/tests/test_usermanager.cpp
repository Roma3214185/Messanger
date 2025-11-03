#include <catch2/catch_all.hpp>

#include <QUrl>
#include <QSignalSpy>
#include <QTest>
#include <QWaitCondition>
#include <QJsonObject>
#include <QByteArray>
#include <QJsonArray>

#include "NetworkManagers/UserManager/usermanager.h"
#include "NetworkAccessManager/MockAccessManager.h"

class TestUserManager : public UserManager {
  public:
    using UserManager::UserManager;

    std::optional<User> onGetUser(QNetworkReply* reply) {
      return UserManager::onGetUser(reply);
    }

    QList<User> onFindUsersByTag(QNetworkReply* reply) {
      return UserManager::onFindUsersByTag(reply);
    }
};


TEST_CASE("Test user manager") {
  MockReply mock_reply;
  MockNetworkAccessManager network_manager(&mock_reply);
  QUrl url("http://localhost:8083/");
  constexpr int times_out = 20;
  UserManager user_manager(&network_manager, url, times_out);
  int user_id = 4;
  auto reply = new MockReply();
  network_manager.setReply(reply);
  User user {
    .id = 1,
    .name = "roma",
    .email = "roma@gmail.com",
    .tag = "roma229",
    .avatarPath = "path/to/avatar"
  };

  QJsonObject obj{
      {"id", user.id},
      {"name", user.name},
      {"email", user.email},
      {"tag", user.tag},
      {"avatar_path", user.avatarPath}
  };
  QJsonDocument doc(obj);
  QByteArray json_data = doc.toJson();


  SECTION("GetUserExpectedRightUrlCreated") {
    QUrl resolved_url("http://localhost:8083/users/4");
    user_manager.getUser(user_id);
    REQUIRE(network_manager.last_request.url() == resolved_url);
  }

  SECTION("GetUserExpectedGetMethod") {
    int before_get_calls = network_manager.get_counter;
    user_manager.getUser(user_id);
    REQUIRE(network_manager.get_counter == before_get_calls + 1);
  }

  SECTION("NoRespondFromServerExpectedEmittedErrorOccured") {
    QSignalSpy errorOccured(&user_manager, &UserManager::errorOccurred);
    int before_calls = errorOccured.count();
    user_manager.getUser(user_id);

    QTRY_COMPARE_WITH_TIMEOUT(errorOccured.count(), before_calls + 1, times_out + 1);
  }

  SECTION("NoRespondFromServerExpectedReturnNullopt") {
    QSignalSpy errorOccured(&user_manager, &UserManager::errorOccurred);
    int before_calls = errorOccured.count();
    auto future = user_manager.getUser(user_id);

    QTRY_COMPARE_WITH_TIMEOUT(errorOccured.count(), before_calls + 1, times_out + 1);
    REQUIRE(future.result() == std::nullopt);
  }

  SECTION("ErrorReplyExpectedEmittedErrorOccuredWithValidText") {
    QSignalSpy spyErrorOccured(&user_manager, &BaseManager::errorOccurred);
    int before_calls = spyErrorOccured.count();
    auto mock_reply = new MockReply();
    mock_reply->setMockError(QNetworkReply::AuthenticationRequiredError, "error");
    network_manager.setReply(mock_reply);

    QTimer::singleShot(0, mock_reply, &MockReply::emitFinished);
    user_manager.getUser(user_id);
    QCoreApplication::processEvents();

    REQUIRE(spyErrorOccured.count() == before_calls + 1);
    auto arguments = spyErrorOccured.takeFirst();
    REQUIRE(arguments.at(0).toString() == "Error occurred: error");
  }

  SECTION("ErrorReplyExpectedReturnedNullopt") {
    auto mock_reply = new MockReply();
    mock_reply->setMockError(QNetworkReply::AuthenticationRequiredError, "error");
    network_manager.setReply(mock_reply);

    QTimer::singleShot(0, mock_reply, &MockReply::emitFinished);
    auto future = user_manager.getUser(user_id);
    QCoreApplication::processEvents();

    std::optional<User> user = future.result();
    REQUIRE(user == std::nullopt);
  }

  SECTION("ValidRespondFromServerExpectedNoErrorOccurred") {
    QSignalSpy spyErrorOccured(&user_manager, &UserManager::errorOccurred);
    int before_calls = spyErrorOccured.count();
    auto mock_reply = new MockReply();
    mock_reply->setData(json_data);
    network_manager.setReply(mock_reply);

    QTimer::singleShot(0, mock_reply, &MockReply::emitFinished);
    user_manager.getUser(user_id);
    QCoreApplication::processEvents();

    REQUIRE(spyErrorOccured.count() == before_calls);
  }

  SECTION("ValidRespondFromServerExpectedValidUserReturned") {
    auto mock_reply = new MockReply();
    mock_reply->setData(json_data);
    network_manager.setReply(mock_reply);

    QTimer::singleShot(0, mock_reply, &MockReply::emitFinished);
    auto future = user_manager.getUser(user_id);
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
  User user {
      .id = 1,
      .name = "roma",
      .email = "roma@gmail.com",
      .tag = "roma229",
      .avatarPath = "path/to/avatar"
  };

  QJsonObject obj{
      {"id", user.id},
      {"name", user.name},
      {"email", user.email},
      {"tag", user.tag},
      {"avatar_path", user.avatarPath}
  };
  QJsonDocument doc(obj);
  QByteArray valid_json_data = doc.toJson();
  MockReply mock_reply;
  MockNetworkAccessManager network_manager(&mock_reply);
  QUrl url("url");
  TestUserManager user_manager(&network_manager, url);

  SECTION("ValidReplyExpectedNotEmittedErrorOccurred") {
    QSignalSpy spyErrorOccured(&user_manager, &UserManager::errorOccurred);
    int before_calls = spyErrorOccured.count();
    auto mock_reply = new MockReply();
    mock_reply->setData(valid_json_data);
    network_manager.setReply(mock_reply);

    user_manager.onGetUser(mock_reply);
    QCoreApplication::processEvents();

    REQUIRE(spyErrorOccured.count() == before_calls);
  }

  SECTION("ValidReplyExpectedReturnedValidResult") {
    auto mock_reply = new MockReply();
    mock_reply->setData(valid_json_data);
    network_manager.setReply(mock_reply);

    auto result = user_manager.onGetUser(mock_reply);
    QCoreApplication::processEvents();

    REQUIRE(result.has_value());
    REQUIRE(result->name == user.name);
    REQUIRE(result->email == user.email);
    REQUIRE(result->tag == user.tag);
    REQUIRE(result->avatarPath == user.avatarPath);
    REQUIRE(result->id == user.id);
  }

  SECTION("NetworkErrorExpectedEmittedErrorOccurredWithText") {
    QSignalSpy spyErrorOccured(&user_manager, &UserManager::errorOccurred);
    int before_calls = spyErrorOccured.count();

    auto mock_reply = new MockReply();
    mock_reply->setMockError(QNetworkReply::ConnectionRefusedError, "connection refused");
    network_manager.setReply(mock_reply);

    auto result = user_manager.onGetUser(mock_reply);
    QCoreApplication::processEvents();

    REQUIRE_FALSE(result.has_value());
    REQUIRE(spyErrorOccured.count() == before_calls + 1);

    auto args = spyErrorOccured.takeFirst();
    REQUIRE(args.at(0).toString() == "get user: connection refused");
  }

  SECTION("InvalidJsonExpectedReturnNulloptWithErrorSignal") {
    QSignalSpy spyErrorOccured(&user_manager, &UserManager::errorOccurred);
    int before_calls = spyErrorOccured.count();

    QByteArray invalid_json_data = "this is not json";
    auto mock_reply = new MockReply();
    mock_reply->setData(invalid_json_data);
    mock_reply->setMockError(QNetworkReply::NoError, "");
    network_manager.setReply(mock_reply);

    auto result = user_manager.onGetUser(mock_reply);
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

    auto mock_reply = new MockReply();
    mock_reply->setData(invalid_json_data);
    mock_reply->setMockError(QNetworkReply::NoError, "");
    network_manager.setReply(mock_reply);

    auto result = user_manager.onGetUser(mock_reply);
    QCoreApplication::processEvents();

    REQUIRE_FALSE(result.has_value());
    REQUIRE(spyErrorOccured.count() == before_calls + 1);
  }

  SECTION("EmptyJsonObjectExpectedReturnNulloptNoErrorSignal") {
    QSignalSpy spyErrorOccured(&user_manager, &UserManager::errorOccurred);
    int before_calls = spyErrorOccured.count();

    QJsonObject emptyObj;
    QJsonDocument emptyDoc(emptyObj);
    QByteArray empty_json_data = emptyDoc.toJson();

    auto mock_reply = new MockReply();
    mock_reply->setData(empty_json_data);
    mock_reply->setMockError(QNetworkReply::NoError, "");
    network_manager.setReply(mock_reply);

    auto result = user_manager.onGetUser(mock_reply);
    QCoreApplication::processEvents();

    REQUIRE(result.has_value());
    REQUIRE(spyErrorOccured.count() == before_calls);
  }
}


TEST_CASE("Test findUsersByTag") {
  MockReply mock_reply;
  MockNetworkAccessManager network_manager(&mock_reply);
  QUrl url("http://localhost:8083/");
  constexpr int times_out = 20;
  UserManager user_manager(&network_manager, url, times_out);
  QString tag = "roma222";
  auto reply = new MockReply();
  network_manager.setReply(reply);

  QList<User> users;
  User user {
      .id = 1,
      .name = "roma",
      .email = "roma@gmail.com",
      .tag = "roma229",
      .avatarPath = "path/to/avatar"
  };

  users.push_back(user);
  users.push_back(user);
  users.push_back(user);
  users.push_back(user);
  users.push_back(user);

  QJsonArray array;
  for(auto user: users) {
    QJsonObject obj{
        {"id", user.id},
        {"name", user.name},
        {"email", user.email},
        {"tag", user.tag},
        {"avatar_path", user.avatarPath}
    };
    array.append(obj);
  }

  QJsonObject root;
  root["users"] = array;

  QJsonDocument doc(root);
  QByteArray json_data = doc.toJson();

  SECTION("ExpectedFucntionCreateRightUrl") {
    user_manager.findUsersByTag(tag);

    REQUIRE(network_manager.last_request.url() == QUrl("http://localhost:8083/users/search?tag=roma222"));
  }

  SECTION("ExpectedGetRequestSended") {
    int before_calls = network_manager.get_counter;
    user_manager.findUsersByTag(tag);

    REQUIRE(network_manager.get_counter == before_calls + 1);
  }

  SECTION("NoRespondFromServerExpectedErrorOccurred") {
    QSignalSpy spyErrorOccurred(&user_manager, &UserManager::errorOccurred);
    int before_calls = spyErrorOccurred.count();
    auto future = user_manager.findUsersByTag(tag);

    QTRY_COMPARE_WITH_TIMEOUT(spyErrorOccurred.count(), before_calls + 1, times_out + 1);

    auto arguments = spyErrorOccurred.takeFirst();
    REQUIRE(arguments.at(0).toString() == "Server didn't respond");
  }

  SECTION("NoRespondFromServerExpectedReturnedEmptyList") {
    QSignalSpy spyErrorOccurred(&user_manager, &UserManager::errorOccurred);
    int before_calls = spyErrorOccurred.count();
    auto future = user_manager.findUsersByTag(tag);

    QTRY_COMPARE_WITH_TIMEOUT(spyErrorOccurred.count(), before_calls + 1, times_out + 1);
    REQUIRE(future.result().isEmpty());
  }

  SECTION("ReturnedErrorFromServerEmpectedEmittedError") {
    auto reply_with_error = new MockReply();
    reply_with_error->setMockError(QNetworkReply::AuthenticationRequiredError, "there is no authentification");
    network_manager.setReply(reply_with_error);
    QSignalSpy spyErrorOccurred(&user_manager, &BaseManager::errorOccurred);
    int before_calls = spyErrorOccurred.count();

    QTimer::singleShot(0, reply_with_error, &MockReply::emitFinished);
    auto future = user_manager.findUsersByTag(tag);
    QCoreApplication::processEvents();

    REQUIRE(spyErrorOccurred.count() == before_calls + 1);
    auto arguments = spyErrorOccurred.takeFirst();
    REQUIRE(arguments.at(0).toString() == "Error occurred: there is no authentification");
  }

  SECTION("ValidReplyExpectedNotEmittedError") {
    auto reply = new MockReply();
    reply->setData(json_data);
    network_manager.setReply(reply);
    QSignalSpy spyErrorOccurred(&user_manager, &UserManager::errorOccurred);
    int before_calls = spyErrorOccurred.count();

    QTimer::singleShot(0, reply, &MockReply::emitFinished);
    auto future = user_manager.findUsersByTag(tag);
    QCoreApplication::processEvents();

    REQUIRE(spyErrorOccurred.count() == before_calls);
  }

  SECTION("GivenInvalidJsonExpectedErrorOccurred") {
    QJsonObject emptyObj;
    QJsonDocument emptyDoc(emptyObj);
    QByteArray empty_json_data = emptyDoc.toJson();

    auto reply = new MockReply();
    reply->setData(empty_json_data);
    network_manager.setReply(reply);
    QSignalSpy spyErrorOccurred(&user_manager, &UserManager::errorOccurred);
    int before_calls = spyErrorOccurred.count();

    QTimer::singleShot(0, reply, &MockReply::emitFinished);
    auto future = user_manager.findUsersByTag(tag);
    QCoreApplication::processEvents();

    REQUIRE(spyErrorOccurred.count() == before_calls);
  }
}


TEST_CASE("Tests onUserFindedByTag") {
  MockReply mock_reply;
  MockNetworkAccessManager network_manager(&mock_reply);
  QUrl url("http://localhost:8083/");
  constexpr int times_out = 20;
  TestUserManager user_manager(&network_manager, url, times_out);
  QString tag = "roma222";
  auto reply = new MockReply();
  network_manager.setReply(reply);

  QList<User> users;
  User user {
      .id = 1,
      .name = "roma",
      .email = "roma@gmail.com",
      .tag = "roma229",
      .avatarPath = "path/to/avatar"
  };

  users.push_back(user);
  users.push_back(user);
  users.push_back(user);
  users.push_back(user);
  users.push_back(user);

  QJsonArray array;
  for(auto user: users) {
    QJsonObject obj{
        {"id", user.id},
        {"name", user.name},
        {"email", user.email},
        {"tag", user.tag},
        {"avatar_path", user.avatarPath}
    };
    array.append(obj);
  }

  QJsonObject root;
  root["users"] = array;

  QJsonDocument doc(root);
  QByteArray valid_json_data = doc.toJson();

  SECTION("GivenValidReplyExpectedNoErrorOccurred") {
    auto reply = new MockReply();
    reply->setData(valid_json_data);
    QSignalSpy spyErrorOccurred(&user_manager, &UserManager::errorOccurred);
    int before_calls = spyErrorOccurred.count();

    user_manager.onFindUsersByTag(reply);

    REQUIRE(spyErrorOccurred.count() == before_calls);
  }

  SECTION("GivenInvalidJsonExpectedErrorOccurred") {
    QJsonObject emptyObj;
    QJsonDocument emptyDoc(emptyObj);
    QByteArray empty_json_data = emptyDoc.toJson();

    auto reply = new MockReply();
    reply->setData(empty_json_data);
    network_manager.setReply(reply);
    QSignalSpy spyErrorOccurred(&user_manager, &UserManager::errorOccurred);
    int before_calls = spyErrorOccurred.count();

    user_manager.onFindUsersByTag(reply);

    REQUIRE(spyErrorOccurred.count() == before_calls);
  }
}

TEST_CASE("UserManager onFindUsersByTag invalid JSON handling") {
  MockReply mock_reply;
  MockNetworkAccessManager network_manager(&mock_reply);
  QUrl url("http://localhost:8083/");
  constexpr int times_out = 20;
  TestUserManager user_manager(&network_manager, url, times_out);
  QString tag = "roma222";
  auto reply = new MockReply();
  network_manager.setReply(reply);

  SECTION("Network error") {
    QSignalSpy spyErrorOccured(&user_manager, &UserManager::errorOccurred);
    auto mock_reply = new MockReply();
    mock_reply->setMockError(QNetworkReply::AuthenticationRequiredError, "network error");

    auto users = user_manager.onFindUsersByTag(mock_reply);
    REQUIRE(users.empty());

    REQUIRE(spyErrorOccured.count() == 1);
    auto arguments = spyErrorOccured.takeFirst();
    REQUIRE(arguments.at(0).toString() == "onFindUsersnetwork error");
  }

  SECTION("Invalid JSON: not a JSON at all") {
    QSignalSpy spyErrorOccured(&user_manager, &UserManager::errorOccurred);
    auto mock_reply = new MockReply();
    mock_reply->setData("this is not json");

    auto users = user_manager.onFindUsersByTag(mock_reply);
    REQUIRE(users.empty());

    REQUIRE(spyErrorOccured.count() == 1);
    auto arguments = spyErrorOccured.takeFirst();
    REQUIRE(arguments.at(0).toString() == "Invalid JSON: expected object at root");
  }

  SECTION("JSON root is an array instead of object") {
    QSignalSpy spyErrorOccured(&user_manager, &UserManager::errorOccurred);
    auto mock_reply = new MockReply();
    mock_reply->setData("[{\"id\":1}]");

    auto users = user_manager.onFindUsersByTag(mock_reply);
    REQUIRE(users.empty());

    REQUIRE(spyErrorOccured.count() == 1);
    auto arguments = spyErrorOccured.takeFirst();
    REQUIRE(arguments.at(0).toString() == "Invalid JSON: expected object at root");
  }

  SECTION("JSON object missing 'users' key") {
    QSignalSpy spyErrorOccured(&user_manager, &UserManager::errorOccurred);
    auto mock_reply = new MockReply();
    mock_reply->setData(R"({"wrong_key": []})");

    auto users = user_manager.onFindUsersByTag(mock_reply);
    REQUIRE(users.empty());

    REQUIRE(spyErrorOccured.count() == 0);
  }

  SECTION("'users' key is not an array") {
    QSignalSpy spyErrorOccured(&user_manager, &UserManager::errorOccurred);
    auto mock_reply = new MockReply();
    mock_reply->setData(R"({"users": 123})");

    auto users = user_manager.onFindUsersByTag(mock_reply);
    REQUIRE(users.empty());

    REQUIRE(spyErrorOccured.count() == 0);
  }

}
