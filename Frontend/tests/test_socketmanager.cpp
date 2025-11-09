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
}
