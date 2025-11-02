#include "catch2/catch_all.hpp"

#include <QUrl>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSignalSpy>
#include <QMetaType>

#include "Managers/SessionManager/sessionmanager.h"
#include "NetworkAccessManager/MockAccessManager.h"
#include "headers/SignUpRequest.h"
#include "headers/User.h"

const QUrl url_auth_service("http://localhost:8083/");
const QUrl url_apigate_service("http://localhost:8084");

class MockReply : public QNetworkReply {
    Q_OBJECT
  public:
    MockReply(QObject* parent = nullptr) : QNetworkReply(parent) {
      open(ReadOnly | Unbuffered);
      setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
      setFinished(true);
      setAttribute(QNetworkRequest::HttpStatusCodeAttribute, 200);
      setUrl(QUrl("http://mock.url"));
    }

    void setMockError(QNetworkReply::NetworkError code, const QString& str) {
      setError(code, str);
    }

    void abort() override {}
    void setData(const QByteArray& data) { this->data = data; }
    void emitFinished() { Q_EMIT finished(); }

    QByteArray data;

  protected:
    qint64 readData(char* buffer, qint64 maxlen) override {
      qint64 len = std::min(maxlen, qint64(data.size()));
      memcpy(buffer, data.constData(), len);
      data.remove(0, len);
      return len;
    }
};

class TestSessionManager : public SessionManager {
  public:
    using SessionManager::SessionManager;

    int on_reply_finished_calls = 0;

    void onReplyFinished(QNetworkReply* reply) override {
      ++on_reply_finished_calls;
      SessionManager::onReplyFinished(reply);
    }
};

TEST_CASE("Test sign in") {
  MockNetworkAccessManager network_manager;
  TestSessionManager session_manager(&network_manager, url_auth_service);
  LogInRequest login_request{"user@test.com", "12345"};

  SECTION("LogInRequestIsSendingWithRightHeader") {
    QUrl resolved_url_auth_service("http://localhost:8083/auth/login");
    session_manager.signIn(login_request);
    REQUIRE(network_manager.last_request.url() == resolved_url_auth_service);
  }

  SECTION("LogInRequestIsSendingOnRightUrl") {
    session_manager.signIn(login_request);
    REQUIRE(network_manager.last_request.header(QNetworkRequest::ContentTypeHeader).toString()
            == "application/json");
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
  MockNetworkAccessManager network_manager;
  TestSessionManager session_manager(&network_manager, url_auth_service);
  SignUpRequest signup_request{
    .email = "user@test.com",
    .password = "12345678",
    .tag = "roma228",
    .name = "roma"
  };

  SECTION("SignUpRequestIsSendingOnRightUrl") {
    QUrl resolved_url_auth_service("http://localhost:8083/auth/register");
    session_manager.signUp(signup_request);
    REQUIRE(network_manager.last_request.url() == resolved_url_auth_service);
  }

  SECTION("SignUpRequestIsSendingWithRightHeader") {
    session_manager.signUp(signup_request);
    REQUIRE(network_manager.last_request.header(QNetworkRequest::ContentTypeHeader).toString()
            == "application/json");
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
  MockNetworkAccessManager network_manager;
  TestSessionManager session_manager(&network_manager, url_auth_service);
  const QString& token = "secret-token123";

  SECTION("authenticateWithTokenExpecteedRightUrl") {
    QUrl resolved_url_auth_service("http://localhost:8083/auth/me");
    session_manager.authenticateWithToken(token);
    REQUIRE(network_manager.last_request.url() == resolved_url_auth_service);
  }

  SECTION("authenticateWithTokenExpectedRightHeader") {
    session_manager.authenticateWithToken(token);
    REQUIRE(network_manager.last_request.header(QNetworkRequest::ContentTypeHeader).toString()
            == "application/json");
  }

  SECTION("authenticateWithTokenExpectedRightRawHeader") {
    session_manager.authenticateWithToken(token);
    QByteArray header_value = network_manager.last_request.rawHeader("Authorization");

    REQUIRE(header_value == token.toUtf8());
  }

  SECTION("authenticateWithTokenExpectedGetCounterIncreadeByOne") {
    int before_post_count = network_manager.get_counter;
    session_manager.authenticateWithToken(token);
    REQUIRE(network_manager.get_counter == before_post_count + 1);
  }

  SECTION("authenticateWithTokenFinishesExpectedEmittingOnAuthenticateWithToken") {
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
  MockNetworkAccessManager network_manager;
  TestSessionManager session_manager(&network_manager, url_auth_service);

  SECTION ("ReplyWithErrorExpectedErrorOccured") {
    auto* reply = new MockReply();
    reply->setMockError(QNetworkReply::ContentNotFoundError, "Invalid parameters");
    QSignalSpy spyErrorOccurred(&session_manager, &TestSessionManager::errorOccurred);
    int before = spyErrorOccurred.count();

    session_manager.onReplyFinished(reply);

    REQUIRE(spyErrorOccurred.count() == before + 1);

    auto arguments = spyErrorOccurred.takeFirst();
    REQUIRE(arguments.at(0).toString() == "Invalid parameters");
  }

  SECTION("ExpectedEmittingUserCreated"){
    auto* reply = new MockReply();
    QByteArray json_data =
      R"({"user":{"name":"ROMA","email":"roma@gmail.com","tag":"r","id":1}, "token":"12345"})";
    reply->setData(json_data);

    QSignalSpy spyUserCreated(&session_manager, &TestSessionManager::userCreated);
    int before = spyUserCreated.count();

    session_manager.onReplyFinished(reply);

    REQUIRE(spyUserCreated.count() == before + 1);
  }

  SECTION("ExpectedEmittingUserCreatedWithRightData"){
    qRegisterMetaType<User>("User");
    auto* reply = new MockReply();
    QByteArray json_data =
        R"({"user":{"name":"ROMA","email":"roma@gmail.com","tag":"r","id":1}, "token":"12345"})";
    reply->setData(json_data);

    QSignalSpy spyUserCreated(&session_manager, &TestSessionManager::userCreated);
    session_manager.onReplyFinished(reply);

    QCoreApplication::processEvents();

    REQUIRE(spyUserCreated.count() == 1);

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
