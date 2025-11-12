#define CATCH_CONFIG_MAIN
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

TEST_CASE("Test presenter communication with other classes", "[presenter]") {
  auto* reply = new MockReply();
  MockNetworkAccessManager netManager(reply);
  FakeSocket fake_socket;
  QUrl url("");
  MockCache cache;
  Model model(url, &netManager, &cache, &fake_socket);
  MockMainWindow window;
  Presenter presenter(&window, &model);

  SECTION("Initialise function tries to check existing token") {
    int before_calls_cash_get = cache.get_calls;
    presenter.initialise();

    REQUIRE(cache.get_calls == before_calls_cash_get + 1);
  }


}
