#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>
#include <QtTest/QSignalSpy>
#include <QtTest/QTest>
#include <catch2/catch_all.hpp>

#include "JsonService.h"
#include "managers/messagemanager.h"
#include "mocks/MockAccessManager.h"
#include "managers/TokenManager.h"

class TestMessageManager : public MessageManager {
 public:
  using MessageManager::MessageManager;
  using MessageManager::onGetChatMessages;
};

TEST_CASE("Test MessageManager getChatMessages") {
  MockReply mock_reply;
  MockNetworkAccessManager network_manager(&mock_reply);
  QUrl url("http://localhost:8081");
  std::chrono::milliseconds timeout_ms = std::chrono::milliseconds{30};
  TokenManager token_manager;
  long long test_current_id = 12345;
  QString token = "secret-token123";
  token_manager.setData(token, test_current_id);
  JsonService entity_factory(&token_manager);
  TestMessageManager message_manager(&entity_factory, &network_manager, url, timeout_ms);
  QJsonArray messages_array{
      QJsonObject{
          {"id", 1}, {"sender_id", 10}, {"chat_id", 12}, {"receiver_id", 14}, {"text", "Hello"}, {"timestamp", "2025-11-03T12:00:00Z"}, {"local_id", "1"}},
      QJsonObject{
          {"id", 2}, {"sender_id", 11}, {"chat_id", 13}, {"receiver_id", 14}, {"text", "Hi"}, {"timestamp", "2025-11-03T12:01:00Z"}, {"local_id", "2"}}};
  QByteArray valid_json = QJsonDocument(messages_array).toJson();

  SECTION("Expected correct endpoint URL with query params") {
    message_manager.getChatMessages("token_abc", 42, 100, 50);
    auto last_url = network_manager.last_request.url();
    REQUIRE(last_url.toString().startsWith("http://localhost:8081/messages/42"));
    REQUIRE(last_url.query().contains("limit=50"));
    REQUIRE(last_url.query().contains("before_id=100"));
  }

  SECTION("No response from server emits timeout error and returns empty list") {
    QSignalSpy spyError(&message_manager, &MessageManager::errorOccurred);
    network_manager.shouldReturnResponce = false;

    auto future = message_manager.getChatMessages("token", 77, 0, 10);
    std::chrono::milliseconds timeout_wait_ms{5};
    QTRY_COMPARE_WITH_TIMEOUT(spyError.count(), 1, timeout_ms + timeout_wait_ms);

    REQUIRE(future.result().isEmpty());
    auto args = spyError.takeFirst();
    REQUIRE(args.at(0).toString() == "Server didn't respond");
  }

  SECTION("Error reply returns empty list and emits proper error") {
    auto reply_with_error = std::make_unique<MockReply>();
    reply_with_error->setMockError(QNetworkReply::ConnectionRefusedError, "connection refused");
    network_manager.setReply(reply_with_error.get());
    network_manager.shouldFail = true;

    QSignalSpy spyError(&message_manager, &MessageManager::errorOccurred);
    // QTimer::singleShot(0, reply_with_error, &MockReply::emitFinished);

    auto future = message_manager.getChatMessages("token", 10, 0, 10);
    QCoreApplication::processEvents();

    REQUIRE(spyError.count() == 1);
    REQUIRE(future.result().isEmpty());

    auto args = spyError.takeFirst();
    REQUIRE(args.at(0).toString().toStdString() == "Error occurred: connection refused");
  }

  SECTION("Invalid JSON (not array) returns empty list and emits error") {
    QByteArray invalid_json = R"({"message": "not an array"})";
    auto reply = std::make_unique<MockReply>();
    reply->setData(invalid_json);
    network_manager.setReply(reply.get());

    QSignalSpy spyError(&message_manager, &MessageManager::errorOccurred);
    // QTimer::singleShot(0, reply, &MockReply::emitFinished);

    auto future = message_manager.getChatMessages("token_xyz", 5, 0, 10);
    QCoreApplication::processEvents();

    auto result = future.result();
    REQUIRE(result.isEmpty());
    REQUIRE(spyError.count() == 1);

    auto args = spyError.takeFirst();
    REQUIRE(args.at(0).toString().contains("Invalid JSON: expected array"));
  }

  SECTION("Valid response returns proper message list") {
    auto reply = std::make_unique<MockReply>();
    reply->setData(valid_json);
    network_manager.setReply(reply.get());

    QSignalSpy spyError(&message_manager, &MessageManager::errorOccurred);
    // QTimer::singleShot(0, reply, &MockReply::emitFinished);

    auto future = message_manager.getChatMessages("token_ok", 42, 0, 2);
    QCoreApplication::processEvents();
    auto messages = future.result();

    REQUIRE(spyError.count() == 0);
    REQUIRE(messages.size() == 2);
    REQUIRE(messages[0].id == 1);
    REQUIRE(messages[0].sender_id == 10);
    REQUIRE(messages[0].getFullText().toStdString() == "Hello");
    REQUIRE(messages[1].id == 2);
    REQUIRE(messages[1].getFullText().toStdString() == "Hi");
  }
}

TEST_CASE("Test MessageManager::onGetChatMessages directly") {
  MockReply mock_reply;
  MockNetworkAccessManager network_manager(&mock_reply);
  QUrl url("http://localhost:8081");
  std::chrono::milliseconds timeout_ms{30};
  TokenManager token_manager;
  long long test_current_id = 12345;
  QString token = "secret-token123";
  token_manager.setData(token, test_current_id);
  JsonService entity_factory(&token_manager);
  TestMessageManager message_manager(&entity_factory, &network_manager, url, timeout_ms);

  SECTION("Invalid JSON emits error and returns empty list") {
    auto reply = std::make_unique<MockReply>();
    reply->setData("not valid json");

    QSignalSpy spyError(&message_manager, &MessageManager::errorOccurred);
    auto result = message_manager.onGetChatMessages(reply->readAll());

    REQUIRE(result.isEmpty());
    REQUIRE(spyError.count() == 1);
    auto args = spyError.takeFirst();
    REQUIRE(args.at(0).toString().contains("Invalid JSON"));
  }

  SECTION("Valid JSON returns correct message list") {
    QJsonArray arr{QJsonObject{{"id", 10}, {"sender_id", 1}, {"text", "msg"}, {"timestamp", "t"}, {"local_id", "3"}}};
    auto reply = std::make_unique<MockReply>();
    reply->setData(QJsonDocument(arr).toJson());

    QSignalSpy spyError(&message_manager, &MessageManager::errorOccurred);
    auto result = message_manager.onGetChatMessages(reply->readAll());

    REQUIRE(spyError.count() == 0);
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].id == 10);
    REQUIRE(result[0].getFullText().toStdString() == "msg");
  }
}
