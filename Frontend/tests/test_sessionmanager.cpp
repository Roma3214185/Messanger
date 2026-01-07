#include <QJsonDocument>
#include <QJsonObject>
#include <QMetaType>
#include <QSignalSpy>
#include <QUrl>
#include <catch2/catch_all.hpp>

#include "dto/SignUpRequest.h"
#include "dto/User.h"
#include "managers/sessionmanager.h"
#include "mocks/MockAccessManager.h"

const QUrl url_auth_service("http://localhost:8083/");
const QUrl url_apigate_service("http://localhost:8084");

class TestSessionManager : public SessionManager {
public:
  using SessionManager::SessionManager;

  int on_reply_finished_calls = 0;

  void onReplyFinished(const QByteArray &responce_data) override {
    ++on_reply_finished_calls;
    SessionManager::onReplyFinished(responce_data);
  }
};

TEST_CASE("Test sign in") {
  MockReply mock_reply;
  MockNetworkAccessManager network_manager(&mock_reply);
  TestSessionManager session_manager(&network_manager, url_auth_service);
  LogInRequest login_request{"user@test.com", "12345"};

  SECTION("LogInRequestIsSendingWithRightHeader") {
    QUrl resolved_url_auth_service("http://localhost:8083/auth/login");
    session_manager.signIn(login_request);
    REQUIRE(network_manager.last_request.url() == resolved_url_auth_service);
  }

  SECTION("LogInRequestIsSendingOnRightUrl") {
    session_manager.signIn(login_request);
    REQUIRE(
        network_manager.last_request.header(QNetworkRequest::ContentTypeHeader)
            .toString() == "application/json");
  }

  SECTION("LogInRequestWithPostCounterIncreadeByOne") {
    int before_post_count = network_manager.post_counter;
    session_manager.signIn(login_request);
    int after_post_count = network_manager.post_counter;
    REQUIRE(after_post_count == before_post_count + 1);
  }

  SECTION("LogInRequestWithRightJsonDocument") {
    session_manager.signIn(login_request);
    QJsonDocument doc = QJsonDocument::fromJson(network_manager.last_data);
    QJsonObject obj = doc.object();
    REQUIRE(obj["email"] == login_request.email);
    REQUIRE(obj["password"] == login_request.password);
  }

  SECTION("LogInFinishesWithEmittingOnSignInFinished") {
    auto reply = new MockReply();
    network_manager.setReply(reply);

    int before_calls = session_manager.on_reply_finished_calls;
    session_manager.signIn(login_request);

    reply->emitFinished();
    QCoreApplication::processEvents();

    REQUIRE(session_manager.on_reply_finished_calls == before_calls + 1);

    delete reply;
  }
}

TEST_CASE("Test sign up") {
  MockReply mock_reply;
  MockNetworkAccessManager network_manager(&mock_reply);
  TestSessionManager session_manager(&network_manager, url_auth_service);
  SignUpRequest signup_request{.email = "user@test.com",
                               .password = "12345678",
                               .tag = "roma228",
                               .name = "roma"};

  SECTION("SignUpRequestIsSendingOnRightUrl") {
    QUrl resolved_url_auth_service("http://localhost:8083/auth/register");
    session_manager.signUp(signup_request);
    REQUIRE(network_manager.last_request.url() == resolved_url_auth_service);
  }

  SECTION("SignUpRequestIsSendingWithRightHeader") {
    session_manager.signUp(signup_request);
    REQUIRE(
        network_manager.last_request.header(QNetworkRequest::ContentTypeHeader)
            .toString() == "application/json");
  }

  SECTION("SignUpRequestWithPostCounterIncreadeByOne") {
    int before_post_count = network_manager.post_counter;
    session_manager.signUp(signup_request);
    int after_post_count = network_manager.post_counter;
    REQUIRE(after_post_count == before_post_count + 1);
  }

  SECTION("SignUpRequestWithRightJsonDocument") {
    session_manager.signUp(signup_request);
    QJsonDocument doc = QJsonDocument::fromJson(network_manager.last_data);
    QJsonObject obj = doc.object();
    REQUIRE(obj["email"] == signup_request.email);
    REQUIRE(obj["password"] == signup_request.password);
    REQUIRE(obj["tag"] == signup_request.tag);
    REQUIRE(obj["name"] == signup_request.name);
  }

  SECTION("SignUpFinishesExpectedEmittingOnSignUpFinished") {
    auto reply = new MockReply();
    network_manager.setReply(reply);

    int before_calls = session_manager.on_reply_finished_calls;
    session_manager.signUp(signup_request);

    reply->emitFinished();
    QCoreApplication::processEvents();

    REQUIRE(session_manager.on_reply_finished_calls == before_calls + 1);

    delete reply;
  }
}

TEST_CASE("Test authenticateWithToken") {
  MockReply mock_reply;
  MockNetworkAccessManager network_manager(&mock_reply);
  TestSessionManager session_manager(&network_manager, url_auth_service);
  const QString &token = "secret-token123";

  SECTION("authenticateWithTokenExpecteedRightUrl") {
    QUrl resolved_url_auth_service("http://localhost:8083/auth/me");
    session_manager.authenticateWithToken(token);
    REQUIRE(network_manager.last_request.url() == resolved_url_auth_service);
  }

  SECTION("authenticateWithTokenExpectedRightHeader") {
    session_manager.authenticateWithToken(token);
    REQUIRE(
        network_manager.last_request.header(QNetworkRequest::ContentTypeHeader)
            .toString() == "application/json");
  }

  SECTION("authenticateWithTokenExpectedRightRawHeader") {
    session_manager.authenticateWithToken(token);
    QByteArray header_value =
        network_manager.last_request.rawHeader("Authorization");

    REQUIRE(header_value == token.toUtf8());
  }

  SECTION("authenticateWithTokenExpectedGetCounterIncreadeByOne") {
    int before_post_count = network_manager.get_counter;
    session_manager.authenticateWithToken(token);
    REQUIRE(network_manager.get_counter == before_post_count + 1);
  }

  SECTION(
      "authenticateWithTokenFinishesExpectedEmittingOnAuthenticateWithToken") {
    auto reply = new MockReply();
    network_manager.setReply(reply);

    int before_calls = session_manager.on_reply_finished_calls;
    session_manager.authenticateWithToken(token);

    reply->emitFinished();
    QCoreApplication::processEvents();

    REQUIRE(session_manager.on_reply_finished_calls == before_calls + 1);

    delete reply;
  }
}

TEST_CASE("Test onSignInFinished") {
  MockReply mock_reply;
  MockNetworkAccessManager network_manager(&mock_reply);
  TestSessionManager session_manager(&network_manager, url_auth_service);

  SECTION("ExpectedEmittingUserCreated") {
    auto *reply = new MockReply();
    QByteArray json_data =
        R"({"user":{"name":"ROMA","email":"roma@gmail.com","tag":"r","id":1}, "token":"12345"})";
    reply->setData(json_data);

    QSignalSpy spyUserCreated(&session_manager,
                              &TestSessionManager::userCreated);
    int before = spyUserCreated.count();

    session_manager.onReplyFinished(reply->readAll());

    REQUIRE(spyUserCreated.count() == before + 1);
  }

  SECTION("ExpectedEmittingUserCreatedWithRightData") {
    qRegisterMetaType<User>("User");
    auto *reply = new MockReply();
    QByteArray json_data =
        R"({"user":{"name":"ROMA","email":"roma@gmail.com","tag":"r","id":1}, "token":"12345"})";
    reply->setData(json_data);

    QSignalSpy spyUserCreated(&session_manager,
                              &TestSessionManager::userCreated);
    int before = spyUserCreated.count();

    session_manager.onReplyFinished(reply->readAll());

    QCoreApplication::processEvents();

    REQUIRE(spyUserCreated.count() == before + 1);

    QList<QVariant> arguments = spyUserCreated.takeFirst();

    User user = arguments.at(0).value<User>();
    QString token = arguments.at(1).toString();

    REQUIRE(user.name == "ROMA");
    REQUIRE(user.email == "roma@gmail.com");
    REQUIRE(user.tag == "r");
    REQUIRE(user.id == 1);
    REQUIRE(token == "12345");

    delete reply;
  }
}

#include "test_sessionmanager.moc"
