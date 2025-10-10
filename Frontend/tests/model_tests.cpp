#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>
#include "../src/Presenter/presenter.h"
#include "../src/headers/IMainWindow.h"
#include "../src/NetworkAccessManager/MockAccessManager.h"
#include "../src/Model/model.h"
#include "QUrl"
#include "headers/MockCash.h"
#include <QCoreApplication>
#include <QtTest/QSignalSpy>
#include <QtTest/QTest>
#include "headers/FakeSocket.h"

TEST_CASE("Test model", "[model]") {
    int argc = 0;
    char* argv[] = {};
    QCoreApplication app(argc, argv);

    MockNetworkAccessManager netManager;
    FakeSocket socket;
    QUrl url("");
    MockCash cash;
    Model model(url, &netManager, &cash, &socket);

    SECTION("GetTokenExpectedNotEmmittedUserCreated"){
        QSignalSpy spyUserCreated(&model, &Model::userCreated);
        int before_calls_spyUserCreated = spyUserCreated.count();
        auto* reply = new MockReply();
        reply->setData(R"({"user":{"name":"ROMA"}})");
        netManager.setReply(reply);

        model.checkToken();

        REQUIRE(spyUserCreated.count() == before_calls_spyUserCreated);
    }

    SECTION("GetTokenExpectedNotEmmittedSpyErrorOcurred"){
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

    SECTION("SignInExpectedUserCreatedEmitted"){
        QByteArray jsonData = R"({"user":{"name":"ROMA","email":"roma@gmail.com","tag":"r","id":1}, "token":"12345"})";
        auto* reply = new MockReply();
        reply->setData(jsonData);
        netManager.setReply(reply);

        QSignalSpy spyUserCreated(&model, &Model::userCreated);
        int before = spyUserCreated.count();

        model.signIn("romanlobach1911@gmail.com", "12345678");
        reply->emitFinished();

        REQUIRE(spyUserCreated.count() == before + 1);
    }

    SECTION("SignInExpectedUserCreatedWithSameInfo"){
        QByteArray jsonData = R"({"user":{"name":"ROMA","email":"roma@gmail.com","tag":"r","id":1}, "token":"12345"})";
        auto* reply = new MockReply();
        reply->setData(jsonData);
        netManager.setReply(reply);

        QSignalSpy spyUserCreated(&model, &Model::userCreated);
        int before = spyUserCreated.count();

        model.signIn("romanlobach1911@gmail.com", "12345678");
        reply->emitFinished();

        QList<QVariant> args = spyUserCreated.takeFirst();
        User emittedUser = args.at(0).value<User>();
        QString token = args.at(1).toString();

        REQUIRE(emittedUser.name == "ROMA");
        REQUIRE(token == "12345");
    }

    SECTION("SignUpExpectedUserCreatedEmitted"){
        QByteArray jsonData = R"({"user":{"name":"ROMA","email":"roma@gmail.com","tag":"r","id":1}, "token":"12345"})";
        auto* reply = new MockReply();
        reply->setData(jsonData);
        netManager.setReply(reply);
        QSignalSpy spyUserCreated(&model, &Model::userCreated);
        int before = spyUserCreated.count();
        SignUpRequest req{
            .email = "romanlobach1911@gmail.com"
        };

        model.signUp(req);

        reply->emitFinished();
        REQUIRE(spyUserCreated.count() == before + 1);
    }


    SECTION("AddTwoSameChatsExpectedNumbersOfChatIncreasedByOne"){
        auto chat = std::make_shared<PrivateChat>();
        chat->chatId = 1;
        int beforeChatsInModel = model.getNumberOfExistingChats();

        model.addChat(chat);
        model.addChat(chat);

        REQUIRE(model.getNumberOfExistingChats() == beforeChatsInModel + 1);
    }

    SECTION("AddTwoSameChatsInDifferentSidesExpectedNumbersOfChatIncreasedByOne"){
        auto chat = std::make_shared<PrivateChat>();
        chat->chatId = 1;
        int beforeChatsInModel = model.getNumberOfExistingChats();

        model.addChat(chat);
        model.addChatInFront(chat);

        REQUIRE(model.getNumberOfExistingChats() == beforeChatsInModel + 1);
    }

    SECTION("SendEmptyMessageExpectedCallsToSocketNotChanged"){
        int before_callsSendText = socket.sendTextMessage_calls;

        model.sendMessage(1, 1, "");

        REQUIRE(socket.sendTextMessage_calls == before_callsSendText);
    }

    SECTION("SendMessageWithOnlyWithGapsExpectedCallsToSocketNotChanged"){
        int before_callsSendText = socket.sendTextMessage_calls;
        QString msgWithGaps = "                                          ";

        model.sendMessage(1, 1, msgWithGaps);

        REQUIRE(socket.sendTextMessage_calls == before_callsSendText);
    }

    SECTION("AfterLogoutExpectedZeroExistingChats"){
        auto chat1 = std::make_shared<PrivateChat>();
        chat1->chatId = 1;
        auto chat2 = std::make_shared<PrivateChat>();
        chat2->chatId = 2;
        model.addChat(chat1);
        model.addChatInFront(chat2);
        REQUIRE(model.getNumberOfExistingChats() == 2);

        model.logout();

        REQUIRE(model.getNumberOfExistingChats() == 0);
    }



}

