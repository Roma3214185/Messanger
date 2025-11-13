#define CATCH_CONFIG_RUNNER
#include <QCoreApplication>
#include <QUrl>
#include <catch2/catch_all.hpp>
#include <QSignalSpy>

#include "MessageListView.h"
#include "interfaces/IMainWindow.h"
#include "mocks/MockAccessManager.h"
#include "model.h"
#include "presenter.h"
#include "mocks/MockMainWindow.h"
#include "mocks/MockCache.h"
#include "mocks/FakeSocket.h"
#include "mocks/MockMessageListView.h"
#include "managers/datamanager.h"

class TestPresenter : public Presenter {
  public:
    using Presenter::Presenter;
    using Presenter::setCurrentChatId;
    using Presenter::newMessage;
};

TEST_CASE("Test presenter communication with other classes", "[presenter]") {
  auto* reply = new MockReply();
  MockNetworkAccessManager netManager(reply);
  FakeSocket fake_socket;
  QUrl url("");
  MockCache cache;
  DataManager data_manager;
  Model model(url, &netManager, &cache, &fake_socket, &data_manager);
  MockMainWindow window;
  MockMessageListView message_list_view(2, 2);
  TestPresenter presenter(&window, &model);
  presenter.setMessageListView(&message_list_view);

  SECTION("Initialise function tries to check existing token") {
    int before_calls_cash_get = cache.get_calls;
    presenter.initialise();

    REQUIRE(cache.get_calls == before_calls_cash_get + 1);
  }

  SECTION("On chat clicked expected window->setChatWindow call") {
    int before_calls = window.call_setChatWindow;
    int chat_id = 4;
    int user_id = 5;
    auto private_chat = ChatFactory::createPrivateChat(chat_id, "Roma", "roma228", user_id, "offline");
    model.addChat(private_chat);

    presenter.onChatClicked(chat_id);

    REQUIRE(window.call_setChatWindow == before_calls + 1);
  }

  SECTION("Add chat expexted emitted signal chat Added") {
    auto private_chat = ChatFactory::createPrivateChat(2, "t", "e", 4, "e");
    QSignalSpy chat_added(&model, &Model::chatAdded);
    int before = chat_added.count();
    MockReply* mockReply = new MockReply();
    netManager.setReply(mockReply);

    model.addChat(private_chat);

    REQUIRE(chat_added.count() == before + 1);
  }

  SECTION("Send message with wothout opened chat expected throw exception") {
    QString message_to_send = "Hi, i'm from test";
    REQUIRE_THROWS(presenter.sendButtonClicked(message_to_send));
  }

  int chat_id = 2;
  int user_id = 4;
  presenter.setCurrentChatId(chat_id);
  auto private_chat = ChatFactory::createPrivateChat(chat_id, "r", "3", user_id, "o");
  data_manager.addChat(private_chat);
  presenter.setId(user_id);

  SECTION("Send message with valid chat_id expected socket gets message to send") {
    QString message_to_send = "Hi, i'm from test";
    int before = fake_socket.sendTextMessage_calls;
    presenter.sendButtonClicked(message_to_send);

    REQUIRE(fake_socket.sendTextMessage_calls == before + 1);
  }

  SECTION("Send empty message expected message won't be send") {
    QString message_to_send = "";
    int before = fake_socket.sendTextMessage_calls;
    presenter.sendButtonClicked(message_to_send);

    REQUIRE(fake_socket.sendTextMessage_calls == before);
  }

  User user;
  user.id = 8;
  auto private_chat4 = ChatFactory::createPrivateChat(chat_id, "r", "3", 8, "2");
  auto message_model = std::make_shared<MessageModel>();
  data_manager.addChat(private_chat4, message_model);
  data_manager.saveUser(user);
  Message first_message;
  first_message.id = 1;
  first_message.text = "Roma";
  first_message.chatId = chat_id;
  first_message.local_id = "1local";
  first_message.timestamp = QDateTime::currentDateTime();
  message_model->addMessage(first_message, user);

  SECTION("Presenter receive new messages expected adding this message to model") {
    Message new_message;
    new_message.id = 4;
    new_message.text = "Roma";
    new_message.chatId = 2;
    new_message.senderId = 8;
    new_message.local_id = "2local";
    int before = message_model->rowCount();

    presenter.newMessage(new_message);

    REQUIRE(message_model->rowCount() == before + 1);
  }

  SECTION("Presenter receive new messages expected this message will be last message") {
    int chat_id = 2;
    Message new_message;
    new_message.id = 4;
    new_message.text = "Roma";
    new_message.chatId = chat_id;
    new_message.senderId = 8;
    new_message.local_id = "2local";
    new_message.timestamp = first_message.timestamp.addDays(1);
    int before = message_model->rowCount();

    presenter.newMessage(new_message);

    REQUIRE(message_model->rowCount() == before + 1);

    auto returned_message = message_model->getLastMessage();
    REQUIRE(returned_message != std::nullopt);
    REQUIRE(returned_message->id == new_message.id);
    REQUIRE(returned_message->text == new_message.text);
    REQUIRE(returned_message->chatId == new_message.chatId);
    REQUIRE(returned_message->senderId == new_message.senderId);
  }

  SECTION("Test presenter receive new messages before the oldest expected this message will be oldest") {
    Message new_message;
    new_message.id = 4;
    new_message.text = "Roma";
    new_message.chatId = 2;
    new_message.senderId = 8;
    new_message.local_id = "2local";
    new_message.timestamp = first_message.timestamp.addDays(-1);
    int before = message_model->rowCount();

    presenter.newMessage(new_message);

    REQUIRE(message_model->rowCount() == before + 1);

    auto returned_message = message_model->getOldestMessage();
    REQUIRE(returned_message != std::nullopt);
    REQUIRE(returned_message->id == new_message.id);
    REQUIRE(returned_message->text == new_message.text);
    REQUIRE(returned_message->chatId == new_message.chatId);
    REQUIRE(returned_message->senderId == new_message.senderId);
  }
}
