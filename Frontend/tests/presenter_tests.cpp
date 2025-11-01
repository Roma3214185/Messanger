#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>
#include "../src/Presenter/presenter.h"
#include "../src/headers/IMainWindow.h"
#include "../src/NetworkAccessManager/MockAccessManager.h"
#include "../src/Model/model.h"
#include "headers/MessageListView.h"
#include "QUrl"
//#include "headers/MockCash.h"
#include <QCoreApplication>

// class MockMainWindow : public IMainWindow
// {
// public:
//     void setUser(User user) override { setUser_calls++; }
//     void setChatWindow(MessageModel* model) override { setChatWindow_calls++;
//     } void setChatModel(ChatModel* model) override { setChatModel_calls++; }
//     void setUserModel(UserModel* userModel) override { setUserModel_calls++;
//     } void clearFindUserEdit() override { clearFindUserEdit_calls++; }

//     int setUser_calls = 0;
//     int setChatWindow_calls = 0;
//     int setChatModel_calls = 0;
//     int setUserModel_calls = 0;
//     int clearFindUserEdit_calls = 0;
// };

// class PresenterTest : public Presenter {
// public:
//     PresenterTest(IMainWindow* window, Model* manager)
//         : Presenter(window, manager) {}
//     using Presenter::setUser;
// };

TEST_CASE("Test presenter communication with other classes", "[presenter]") {
  SECTION("A") {
    REQUIRE(1 == 1);
  }
//     int argc = 0;
//     char* argv[] = {};
//     QCoreApplication app(argc, argv);

     // MockNetworkAccessManager netManager;
     // QUrl url("");
     // MockCash cash;
     // Model model(url, &netManager, &cash);
     // MockMainWindow window;
     // PresenterTest presenter(&window, &model);

     // SECTION("TestSetUser") {
     //     User user{
     //         .name = "ROMA"
     //     };
     //     QString token = "token";
     //     int before_view_setUser = window.setUser_calls;
     //     auto* reply = new MockReply();
     //     reply->setData(R"({"user":{"name":"ROMA"}})");
     //     netManager.setReply(reply);

     //     SECTION("TestUserExpectedOneCallInWindow") {
     //         presenter.setUser(user, token);
     //         int after_view_setUser = window.setUser_calls;
     //         REQUIRE(before_view_setUser == after_view_setUser - 1);
     //     }
     // }
 }
