#define CATCH_CONFIG_RUNNER
#include <QCoreApplication>
#include <QUrl>
#include <catch2/catch_all.hpp>

#include "MessageListView.h"
#include "interfaces/IMainWindow.h"
#include "mocks/MockAccessManager.h"
#include "model.h"
#include "presenter.h"
#include "mocks/MockMainWindow.h"
#include "mocks/MockCache.h"
#include "mocks/FakeSocket.h"
#include "mocks/MockMessageListView.h"

TEST_CASE("Test presenter communication with other classes", "[presenter]") {
  auto* reply = new MockReply();
  MockNetworkAccessManager netManager(reply);
  FakeSocket fake_socket;
  QUrl url("");
  MockCache cache;
  Model model(url, &netManager, &cache, &fake_socket);
  MockMainWindow window;
  MockMessageListView message_list_view;
  Presenter presenter(&window, &model);
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




}
