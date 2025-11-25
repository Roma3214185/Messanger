#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>
#include <QtTest/QSignalSpy>
#include <QtTest/QTest>
#include <catch2/catch_all.hpp>

#include "JsonService.h"
#include "managers/chatmanager.h"
#include "mocks/MockAccessManager.h"

class TestChatManager : public ChatManager {
 public:
  using ChatManager::ChatManager;

  QList<ChatPtr> onLoadChats(QNetworkReply* reply) { return ChatManager::onLoadChats(reply); }

  ChatPtr onChatLoaded(QNetworkReply* reply) { return ChatManager::onChatLoaded(reply); }

  ChatPtr onCreatePrivateChat(QNetworkReply* reply) {
    return ChatManager::onCreatePrivateChat(reply);
  }
};

TEST_CASE("Test ChatManager loadChats") {
  MockReply                mock_reply;
  MockNetworkAccessManager network_manager(&mock_reply);
  QUrl                     url("http://localhost:8081");
  constexpr int            times_out = 20;
  TestChatManager          chat_manager(&network_manager, url, times_out);
  QJsonObject              chat_obj{
                   {"type", "private"},
                   {"id", 43},
                   {"user", QJsonObject{{"id", 123}, {"name", "Chat43"}, {"avatar", "path/to/avatar"}}}};

  QJsonArray chat_array;
  chat_array.append(chat_obj);

  QJsonObject root{{"chats", chat_array}};
  QByteArray  valid_json_data = QJsonDocument(root).toJson();

  SECTION("Expected correct endpoint URL") {
    chat_manager.loadChats("token");
    REQUIRE(network_manager.last_request.url() == QUrl("http://localhost:8081/chats"));
  }

  SECTION("No response from server emits error and returns empty list") {
    QSignalSpy spyError(&chat_manager, &ChatManager::errorOccurred);
    network_manager.shouldReturnResponce = false;

    auto       future = chat_manager.loadChats("token");

    QTRY_COMPARE_WITH_TIMEOUT(spyError.count(), 1, times_out + 1);
    REQUIRE(future.result().isEmpty());
  }

  SECTION("Error reply returns empty list and emits proper error") {
    auto reply_with_error = new MockReply();
    reply_with_error->setMockError(QNetworkReply::AuthenticationRequiredError, "auth failed");
    network_manager.setReply(reply_with_error);

    QSignalSpy spyError(&chat_manager, &ChatManager::errorOccurred);
    network_manager.shouldFail = true;
    //QTimer::singleShot(0, reply_with_error, &MockReply::emitFinished);
    auto future = chat_manager.loadChats("token");
    QCoreApplication::processEvents();

    REQUIRE(spyError.count() == 1);
    REQUIRE(future.result().isEmpty());

    auto args = spyError.takeFirst();
    REQUIRE(args.at(0).toString().toStdString() == "Error occurred: auth failed");
  }

  SECTION("Valid response returns proper chats") {
    auto reply = new MockReply();
    reply->setData(valid_json_data);
    network_manager.setReply(reply);
    //QTimer::singleShot(0, reply, &MockReply::emitFinished);

    auto future = chat_manager.loadChats("token");
    QCoreApplication::processEvents();
    auto chats = future.result();

    REQUIRE(chats.size() == 1);
    REQUIRE(chats[0]->chat_id == 43);
    REQUIRE(chats[0]->title == "Chat43");
    REQUIRE(chats[0]->avatar_path == "path/to/avatar");
  }

  SECTION("Invalid JSON returns empty list with error") {
    QByteArray invalid_json = "not json";
    auto       reply        = new MockReply();
    reply->setData(invalid_json);
    network_manager.setReply(reply);

    QSignalSpy spyError(&chat_manager, &ChatManager::errorOccurred);
    //QTimer::singleShot(0, reply, &MockReply::emitFinished);

    auto future = chat_manager.loadChats("token");
    QCoreApplication::processEvents();

    REQUIRE(future.result().isEmpty());
    REQUIRE(spyError.count() == 1);
  }
}

TEST_CASE("Test ChatManager loadChat") {
  MockReply                mock_reply;
  MockNetworkAccessManager network_manager(&mock_reply);
  QUrl                     url("http://localhost:8081");
  constexpr int            times_out = 20;
  TestChatManager          chat_manager(&network_manager, url, times_out);

  QJsonObject chat_obj{
      {"type", "private"},
      {"id", 42},
      {"user", QJsonObject{{"id", 123}, {"name", "Chat42"}, {"avatar", "path/to/avatar"}}}};

  QByteArray valid_json = QJsonDocument(chat_obj).toJson();

  SECTION("Expected correct URL for chat ID") {
    chat_manager.loadChat("token", 42);
    REQUIRE(network_manager.last_request.url() == QUrl("http://localhost:8081/chats/42"));
  }

  SECTION("Error reply returns nullptr and emits error") {
    auto reply_with_error = new MockReply();
    reply_with_error->setMockError(QNetworkReply::ConnectionRefusedError, "connection refused");
    network_manager.setReply(reply_with_error);

    QSignalSpy spyError(&chat_manager, &ChatManager::errorOccurred);
    network_manager.shouldFail = true;
    //QTimer::singleShot(0, reply_with_error, &MockReply::emitFinished);

    auto future = chat_manager.loadChat("token", 42);
    QCoreApplication::processEvents();

    REQUIRE(future.result() == nullptr);
    REQUIRE(spyError.count() == 1);
    auto args = spyError.takeFirst();
    REQUIRE(args.at(0).toString().toStdString() == "Error occurred: connection refused");
  }

  SECTION("Valid response returns proper chat object") {
    auto reply = new MockReply();
    reply->setData(valid_json);
    network_manager.setReply(reply);
    //QTimer::singleShot(0, reply, &MockReply::emitFinished);

    auto future = chat_manager.loadChat("token", 42);
    QCoreApplication::processEvents();
    auto chat = future.result();
    REQUIRE(chat != nullptr);
    REQUIRE(chat->chat_id == 42);
    REQUIRE(chat->title == "Chat42");
  }

  SECTION("Invalid JSON returns nullptr and emits error") {
    auto reply = new MockReply();
    reply->setData("not json");
    network_manager.setReply(reply);

    QSignalSpy spyError(&chat_manager, &ChatManager::errorOccurred);
    //QTimer::singleShot(0, reply, &MockReply::emitFinished);

    auto future = chat_manager.loadChat("token", 42);
    QCoreApplication::processEvents();

    REQUIRE(future.result() == nullptr);
    REQUIRE(spyError.count() == 1);
  }
}

TEST_CASE("Test ChatManager createPrivateChat") {
  MockReply                mock_reply;
  MockNetworkAccessManager network_manager(&mock_reply);
  QUrl                     url("http://localhost:8081");
  constexpr int            times_out = 20;
  TestChatManager          chat_manager(&network_manager, url, times_out);

  QJsonObject user_obj{
      {"id", 55},
      {"name", "PrivateChat101"},
      {"avatar", "/path/to/avatar.png"}
  };

  QJsonObject chat_obj{
      {"type", "private"},
      {"id", 101},
      {"user", user_obj}
  };

  QByteArray  valid_json = QJsonDocument(chat_obj).toJson();

  SECTION("Correct POST request URL and body") {
    chat_manager.createPrivateChat("token", 5);
    REQUIRE(network_manager.last_request.url() == QUrl("http://localhost:8081/chats/private"));
  }

  SECTION("Error reply returns nullptr and emits error") {
    auto reply_with_error = new MockReply();
    reply_with_error->setMockError(QNetworkReply::ConnectionRefusedError, "connection refused");
    network_manager.setReply(reply_with_error);

    QSignalSpy spyError(&chat_manager, &ChatManager::errorOccurred);
    //QTimer::singleShot(0, reply_with_error, &MockReply::emitFinished);

    auto future = chat_manager.createPrivateChat("token", 5);
    QCoreApplication::processEvents();

    REQUIRE(future.result() == nullptr);
    REQUIRE(spyError.count() == 1);
  }

  SECTION("Valid private chat response returns chat object") {
    auto reply = new MockReply();
    reply->setData(valid_json);
    network_manager.setReply(reply);
    //QTimer::singleShot(0, reply, &MockReply::emitFinished);

    auto future = chat_manager.createPrivateChat("token", 5);
    QCoreApplication::processEvents();

    auto chat = future.result();
    REQUIRE(chat != nullptr);
    REQUIRE(chat->chat_id == 101);
    REQUIRE(chat->title == "PrivateChat101");
  }

  SECTION("Invalid JSON returns nullptr and emits error") {
    auto reply = new MockReply();
    reply->setData("not json");
    network_manager.setReply(reply);

    QSignalSpy spyError(&chat_manager, &BaseManager::errorOccurred);
    int        before = spyError.count();
    //QTimer::singleShot(0, reply, &MockReply::emitFinished);

    auto future = chat_manager.createPrivateChat("token", 5);
    QCoreApplication::processEvents();
    auto res = future.result();

    REQUIRE(res == nullptr);
    REQUIRE(spyError.count() == before + 1);
  }

  SECTION("Chat type is not PRIVATE returns nullptr and emits error") {
    QJsonObject invalid_chat_obj{{"chat_id", 102}, {"chat_type", "GROUP"}, {"name", "GroupChat"}};
    auto        reply = new MockReply();
    reply->setData(QJsonDocument(invalid_chat_obj).toJson());
    network_manager.setReply(reply);
    QSignalSpy spyError(&chat_manager, &BaseManager::errorOccurred);

    //QTimer::singleShot(0, reply, &MockReply::emitFinished);
    auto future = chat_manager.createPrivateChat("token", 5);
    QCoreApplication::processEvents();

    REQUIRE(future.result() == nullptr);
    REQUIRE(spyError.count() == 1);
    auto args = spyError.takeFirst();
    REQUIRE(args.at(0).toString() == "Error in model create private chat returned group chat");
  }

  SECTION("On load chats receive reply with error expected emit ErrorOccurred with right message") {
    QString mock_error_message = "connection refused";
    auto reply_with_error = new MockReply();
    reply_with_error->setMockError(QNetworkReply::ConnectionRefusedError, mock_error_message);

    QSignalSpy spyError(&chat_manager, &ChatManager::errorOccurred);
    //QTimer::singleShot(0, reply_with_error, &MockReply::emitFinished);

    chat_manager.onLoadChats(reply_with_error);

    REQUIRE(spyError.count() == 1);

    auto args = spyError.takeFirst();
    REQUIRE(args.at(0).toString() == mock_error_message);
  }

  SECTION("On chat loaded receive reply with error expected emit ErrorOccurred with right message") {
    QString mock_error_message = "error occured";
    auto reply_with_error = new MockReply();
    reply_with_error->setMockError(QNetworkReply::ConnectionRefusedError, mock_error_message);

    QSignalSpy spyError(&chat_manager, &ChatManager::errorOccurred);
    //QTimer::singleShot(0, reply_with_error, &MockReply::emitFinished);

    chat_manager.onChatLoaded(reply_with_error);

    REQUIRE(spyError.count() == 1);

    auto args = spyError.takeFirst();
    REQUIRE(args.at(0).toString() == mock_error_message);
  }
}
