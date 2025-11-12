#include <QJsonArray>
#include <QSignalSpy>
#include <catch2/catch_all.hpp>

#include "JsonService.h"
#include "managers/socketmanager.h"
#include "mocks/FakeSocket.h"

TEST_CASE("Test socket") {
  FakeSocket    fakesocket;
  QUrl          url("http://localhost:8086/");
  SocketManager socket_manager(&fakesocket, url);
  int           user_id = 10;

  SECTION("Connect socket expected to call socket open") {
    int before_calls = fakesocket.open_calls;
    socket_manager.connectSocket();
    REQUIRE(fakesocket.open_calls == before_calls + 1);
  }

  SECTION("Connect socket connect for right url") {
    QUrl url_socket("ws://localhost:8086/ws");
    socket_manager.connectSocket();
    REQUIRE(fakesocket.last_opened_url.toString() == url_socket.toString());
  }

  SECTION("NewMessageExpectedEmittedOnNewMessage") {
    QSignalSpy spyNewMessage(&socket_manager, &SocketManager::newTextFromSocket);
    int        before          = spyNewMessage.count();
    QString    message_to_send = "Hi Roma";
    // socket_manager.connectSocket(20);
    fakesocket.receiveTextMessage(message_to_send);

    QCoreApplication::processEvents();

    REQUIRE(spyNewMessage.count() == before + 1);

    QList<QVariant> arguments           = spyNewMessage.takeFirst();
    QString         message_from_socket = arguments.at(0).toString();

    REQUIRE(message_from_socket == message_to_send);
  }

  SECTION("Socket is sending init message expected send message") {
    int user_id = 2;
    int before = fakesocket.sendTextMessage_calls;
    socket_manager.initSocket(user_id);

    REQUIRE(fakesocket.sendTextMessage_calls == before + 1);

    auto sended_message = fakesocket.last_sended_message;

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(sended_message.toUtf8(), &parseError);

    REQUIRE(parseError.error == QJsonParseError::NoError);
    REQUIRE(doc.isObject());

    QJsonObject obj = doc.object();

    REQUIRE(obj.contains("type"));
    REQUIRE(obj.contains("user_id"));
    REQUIRE(obj["type"].toString() == "init");
    REQUIRE(obj["user_id"].toInt() == user_id);
  }

  SECTION("Socket close expected calls close() and disconnect()") {
    int before_close_calls = fakesocket.close_calls;
    int before_disconnect_calls = fakesocket.disconnect_calls;

    socket_manager.close();

    REQUIRE(fakesocket.close_calls == before_close_calls + 1);
    REQUIRE(fakesocket.disconnect_calls == before_disconnect_calls + 1);
  }

  SECTION("Socket send message") {
    QString message_to_send = "Hi!This is test message";

    socket_manager.sendText(message_to_send);

    REQUIRE(fakesocket.last_sended_message == message_to_send);
  }

}
