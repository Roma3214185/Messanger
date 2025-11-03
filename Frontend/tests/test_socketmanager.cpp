#include <catch2/catch_all.hpp>

#include <QJsonArray>
#include <QSignalSpy>

#include "headers/JsonService.h"
#include "headers/FakeSocket.h"
#include "SocketManager/socketmanager.h"

class TestSocketManager : public SocketManager {
  public:
    using SocketManager::SocketManager;

    int on_socket_connected_calls = 0;

    void onSocketConnected(int user_id) {
      ++on_socket_connected_calls;
      SocketManager::onSocketConnected(user_id);
    }
};

TEST_CASE("Test socket") {
  FakeSocket fakesocket;
  QUrl url("http://localhost:8086/");
  TestSocketManager socket_manager(&fakesocket, url);
  int user_id = 10;

  SECTION("Connect socket expected to call socket open") {
    int before_calls = fakesocket.open_calls;
    socket_manager.connectSocket(user_id);
    REQUIRE(fakesocket.open_calls == before_calls + 1);
  }

  SECTION("Connect socket connect for right url") {
    QUrl url_socket("ws://localhost:8086/ws");
    socket_manager.connectSocket(user_id);
    REQUIRE(fakesocket.last_opened_url.toString() == url_socket.toString());
  }

  SECTION("onSocketConnectedExpectedToSendInitMessageToSocket") {
    int before_calls = socket_manager.on_socket_connected_calls;
    socket_manager.onSocketConnected(2);

    REQUIRE(socket_manager.on_socket_connected_calls == before_calls + 1);
  }

  SECTION("onSocketConnectedExpectedToSendInitRightMessageToSocket") {
    int before_calls = socket_manager.on_socket_connected_calls;
    int user_id = 2;
    socket_manager.onSocketConnected(user_id);
    REQUIRE(socket_manager.on_socket_connected_calls == before_calls + 1);


    QJsonDocument doc = QJsonDocument::fromJson(fakesocket.last_sended_message.toUtf8());
    REQUIRE(!doc.isNull());
    REQUIRE(doc.isObject());

    QJsonObject obj = doc.object();
    REQUIRE(obj["type"].toString() == "init");
    REQUIRE(obj["user_id"].toInt() == user_id);
  }

  SECTION("NewMessageExpectedEmittedOnNewMessage") {
    QSignalSpy spyNewMessage(&socket_manager, &SocketManager::newTextFromSocket);
    int before = spyNewMessage.count();
    QString message_to_send = "Hi Roma";
    //socket_manager.connectSocket(20);
    fakesocket.receiveTextMessage(message_to_send);

    QCoreApplication::processEvents();

    REQUIRE(spyNewMessage.count() == before + 1);

    QList<QVariant> arguments = spyNewMessage.takeFirst();
    QString message_from_socket = arguments.at(0).toString();

    REQUIRE(message_from_socket == message_to_send);
  }
}
