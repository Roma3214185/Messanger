//#define CATCH_CONFIG_MAIN
#include <QCoreApplication>
#include <QUrl>
#include <QtTest/QSignalSpy>
#include <QtTest/QTest>
#include <catch2/catch_all.hpp>

#include "Model/model.h"
#include "NetworkAccessManager/MockAccessManager.h"
#include "Presenter/presenter.h"
#include "headers/FakeSocket.h"
#include "headers/IMainWindow.h"
#include "headers/MockCache.h"
#include "headers/SignUpRequest.h"
#include "headers/MessageListView.h"

#include <QNetworkReply>
#include <QObject>

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

TEST_CASE("Test model", "[model]") {
  MockNetworkAccessManager netManager;
  FakeSocket socket;
  QUrl url("");
  MockCache cash;
  Model model(url, &netManager, &cash, &socket);

  SECTION("GetTokenExpectedNotEmmittedUserCreated") {
    QSignalSpy spyUserCreated(&model, &Model::userCreated);
    int before_calls_spyUserCreated = spyUserCreated.count();
    auto* reply = new MockReply();
    reply->setData(R"({"user":{"name":"ROMA"}})");
    netManager.setReply(reply);

    model.checkToken();

    REQUIRE(spyUserCreated.count() == before_calls_spyUserCreated);
  }

  SECTION("GetTokenExpectedNotEmmittedSpyErrorOcurred") {
    QSignalSpy spyErrorOcurred(&model, &Model::errorOccurred);
    int before_calls_spyErrorOcurred = spyErrorOcurred.count();
    auto* reply = new MockReply();
    reply->setData(R"({"user":{"name":"ROMA"}})");
    netManager.setReply(reply);

    model.checkToken();

    REQUIRE(spyErrorOcurred.count() == before_calls_spyErrorOcurred);
  }

  SECTION("SaveTokenExpectedCashReturnToken") {
    model.saveToken("secret-token");

    auto tok = cash.get("TOKEN");

    REQUIRE(tok != std::nullopt);
    REQUIRE(*tok == "secret-token");
  }

  SECTION("DeleteTokenExpectedCashReturnNullopt") {
    cash.saveToken("TOKEN", "secret-token");
    model.deleteToken();

    auto tok = cash.get("TOKEN");

    REQUIRE(tok == std::nullopt);
  }

  SECTION("SignInExpectedUserCreatedEmitted") {
    QByteArray jsonData =
        R"({"user":{"name":"ROMA","email":"roma@gmail.com","tag":"r","id":1}, "token":"12345"})";
    auto* reply = new MockReply();
    reply->setData(jsonData);
    netManager.setReply(reply);

    QSignalSpy spyUserCreated(&model, &Model::userCreated);
    int before = spyUserCreated.count();
    LogInRequest login_request{ .email = "romanlobach1911@gmail.com", .password = "12345678"};
    model.signIn(login_request);
    reply->emitFinished();

    REQUIRE(spyUserCreated.count() == before + 1);
  }

  SECTION("SignInExpectedUserCreatedWithSameInfo") {
    QByteArray jsonData =
        R"({"user":{"name":"ROMA","email":"roma@gmail.com","tag":"r","id":1}, "token":"12345"})";
    auto* reply = new MockReply();
    reply->setData(jsonData);
    netManager.setReply(reply);

    QSignalSpy spyUserCreated(&model, &Model::userCreated);
    int before = spyUserCreated.count();
    LogInRequest login_request{ .email = "romanlobach1911@gmail.com", .password = "12345678"};
    model.signIn(login_request);
    reply->emitFinished();

    QList<QVariant> args = spyUserCreated.takeFirst();
    User emittedUser = args.at(0).value<User>();
    QString token = args.at(1).toString();

    REQUIRE(emittedUser.name == "ROMA");
    REQUIRE(token == "12345");
  }

  SECTION("SignUpExpectedUserCreatedEmitted") {
    QByteArray jsonData =
        R"({"user":{"name":"ROMA","email":"roma@gmail.com","tag":"r","id":1}, "token":"12345"})";
    auto* reply = new MockReply();
    reply->setData(jsonData);
    netManager.setReply(reply);
    QSignalSpy spyUserCreated(&model, &Model::userCreated);
    int before = spyUserCreated.count();
    SignUpRequest req{.email = "romanlobach1911@gmail.com"};

    model.signUp(req);

    reply->emitFinished();
    REQUIRE(spyUserCreated.count() == before + 1);
  }

  SECTION("AddTwoSameChatsExpectedNumbersOfChatIncreasedByOne") {
    auto chat = std::make_shared<PrivateChat>();
    chat->chat_id = 1;
    int beforeChatsInModel = model.getNumberOfExistingChats();

    model.addChat(chat);
    model.addChat(chat);

    REQUIRE(model.getNumberOfExistingChats() == beforeChatsInModel + 1);
  }

  SECTION(
      "AddTwoSameChatsInDifferentSidesExpectedNumbersOfChatIncreasedByOne") {
    auto chat = std::make_shared<PrivateChat>();
    chat->chat_id = 1;
    int beforeChatsInModel = model.getNumberOfExistingChats();

    model.addChat(chat);
    model.addChatInFront(chat);

    REQUIRE(model.getNumberOfExistingChats() == beforeChatsInModel + 1);
  }

  Message message;
  message.senderId = 1;
  message.chatId = 1;

  SECTION("SendEmptyMessageExpectedCallsToSocketNotChanged") {
    int before_callsSendText = socket.sendTextMessage_calls;
    message.text = "";
    model.sendMessage(message);

    REQUIRE(socket.sendTextMessage_calls == before_callsSendText);
  }

  SECTION("SendMessageWithOnlyWithGapsExpectedCallsToSocketNotChanged") {
    int before_callsSendText = socket.sendTextMessage_calls;
    message.text = "                   ";

    model.sendMessage(message);

    REQUIRE(socket.sendTextMessage_calls == before_callsSendText);
  }

  SECTION("AfterLogoutExpectedZeroExistingChats") {
    auto chat1 = std::make_shared<PrivateChat>();
    chat1->chat_id = 1;
    auto chat2 = std::make_shared<PrivateChat>();
    chat2->chat_id = 2;
    model.addChat(chat1);
    model.addChatInFront(chat2);
    REQUIRE(model.getNumberOfExistingChats() == 2);

    model.logout();

    REQUIRE(model.getNumberOfExistingChats() == 0);
  }
}

#include "model_tests.moc"

