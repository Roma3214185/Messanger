#include <catch2/catch_all.hpp>

#include <QJsonArray>

#include "Managers/MessageManager/messagemanager.h"
#include "headers/INetworkAccessManager.h"
#include "NetworkAccessManager/MockAccessManager.h"
#include "headers/MockReply.h"
#include "headers/JsonService.h"

class TestMessageManager : public MessageManager {
  public:
    using MessageManager::MessageManager;

    int on_get_chat_messages_calls = 0;

    QList<Message> onGetChatMessages(QNetworkReply* reply) {
      ++on_get_chat_messages_calls;
      return MessageManager::onGetChatMessages(reply);
    }
};

TEST_CASE("Test getChatMessages") {
  MockNetworkAccessManager network_manager;
  QUrl url("http://localhost:8082/");
  TestMessageManager message_manager(&network_manager, url);
  QString token = "secret-token";
  int chat_id = 2;
  int before_id = 3;
  int limit = 20;
  auto reply = new MockReply();
  network_manager.setReply(reply);
  std::vector<Message> messages;
  messages.push_back(Message{1, 1, 1, "1"});
  messages.push_back(Message{2, 2, 2, "2"});
  messages.push_back(Message{3, 3, 3, "3"});
  messages.push_back(Message{4, 4, 4, "4"});

  SECTION("GetChatMessagesExpectedCreatingRightUrl") {
    QUrl right_resolved_url = QUrl("http://localhost:8082/messages/2?limit=20&beforeId=3");
    QTimer::singleShot(0, reply, &QNetworkReply::finished);

    message_manager.getChatMessages(token, chat_id, before_id, limit);

    REQUIRE(network_manager.last_request.url() == right_resolved_url);
  }

  SECTION("TestOnGetChatMessagesExpectedSameResultSize") {
    QJsonArray jsonArray;
    for(auto message: messages) {
      QJsonObject msg_json;
      msg_json["id"] = message.id;
      msg_json["text"] = message.text;
      jsonArray.append(msg_json);
    }
    QJsonDocument doc(jsonArray);
    QByteArray data = doc.toJson(QJsonDocument::Compact);
    auto new_reply = new MockReply();
    new_reply->setData(data);

    auto res = message_manager.onGetChatMessages(new_reply);

    REQUIRE(res.size() == messages.size());
  }
}
