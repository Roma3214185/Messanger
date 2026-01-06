// #include <QUrl>
// #include <catch2/catch_all.hpp>
// #include <QSignalSpy>
// #include <QJsonObject>

// #include "MessageListView.h"
// #include "interfaces/IMainWindow.h"
// #include "mocks/MockAccessManager.h"
// #include "model.h"
// #include "presenter.h"
// #include "mocks/MockMainWindow.h"
// #include "mocks/MockCache.h"
// #include "mocks/FakeSocket.h"
// #include "mocks/MockMessageListView.h"
// #include "managers/datamanager.h"
// #include "dto/SignUpRequest.h"

// class TestPresenter : public Presenter {
//   public:
//     using Presenter::Presenter;
//     using Presenter::setCurrentChatId;
//     using Presenter::newMessage;
//     using Presenter::onNewResponce;

//     void setMockCurrentUser(User user) {
//       Presenter::current_user_ = user;
//     }

//     void setMockOpenedChatId(int chat_id) {
//        Presenter::current_opened_chat_id_ = chat_id;
//     }
// };

// struct TestPresenterFixrute {
//     std::unique_ptr<MockReply> reply;
//     MockNetworkAccessManager netManager;
//     FakeSocket fake_socket;
//     QUrl url;
//     MockCache cache;
//     DataManager data_manager;
//     Model model;
//     MockMainWindow window;
//     MockMessageListView message_list_view;
//     TestPresenter presenter;

//     QString email = "roma-test-email@gmail.com";
//     QString password = "password-test-password";
//     QString tag = "test-roma-228-tag";
//     QString name = "test-name";

//     TestPresenterFixrute()
//         : reply(std::make_unique<MockReply>())
//         , netManager(reply.get())
//         , model(url, &netManager, &cache, &fake_socket, &data_manager)
//         , message_list_view(2, 2)
//         , presenter(&window, &model) {
//       presenter.setMessageListView(&message_list_view);
//     }
// };

// //TODO: extract check login from mainwindow to session-manager/model???

// TEST_CASE("Test presenter communication with other classes", "[presenter]") {
//   TestPresenterFixrute fix;

//   SECTION("Initialise function tries to check existing token") {
//     int before_calls_cash_get = fix.cache.get_calls;
//     fix.presenter.initialise();

//     REQUIRE(fix.cache.get_calls == before_calls_cash_get + 1);
//   }

//   SECTION("Presenter receive responce but currend_user_id == nullopt expected
//   throw exception") {
//     QJsonObject json;
//     json["type"] = "opened";

//     REQUIRE_THROWS(fix.presenter.onNewResponce(json));
//   }

//   SECTION("On chat clicked expected window->setChatWindow call") {
//     int before_calls = fix.window.call_setChatWindow;
//     int chat_id = 4;
//     int user_id = 5;
//     auto private_chat = ChatFactory::createPrivateChat(chat_id, "Roma",
//     "roma228", user_id, "offline"); fix.model.addChat(private_chat);

//     fix.presenter.onChatClicked(chat_id);

//     REQUIRE(fix.window.call_setChatWindow == before_calls + 1);
//   }
// }

// TEST_CASE("Presenter handle CRUD operations") {
//   TestPresenterFixrute fix;

//   SECTION("Add chat expexted emitted signal chat Added") {
//     auto private_chat = ChatFactory::createPrivateChat(2, "t", "e", 4, "e");
//     QSignalSpy chat_added(&fix.model, &Model::chatAdded);
//     int before = chat_added.count();
//     MockReply* mockReply = new MockReply();
//     fix.netManager.setReply(mockReply);

//     fix.model.addChat(private_chat);

//     REQUIRE(chat_added.count() == before + 1);
//   }

//   SECTION("Send message without opened chat expected no send message") {
//     QString message_to_send = "Hi, i'm from test";
//     int before = fix.fake_socket.sendTextMessage_calls;
//     fix.presenter.sendButtonClicked(message_to_send);

//     REQUIRE(fix.fake_socket.sendTextMessage_calls == before);
//   }

//   int chat_id = 2;
//   int user_id = 4;
//   fix.presenter.setCurrentChatId(chat_id);
//   auto private_chat = ChatFactory::createPrivateChat(chat_id, "r", "3",
//   user_id, "o"); fix.data_manager.addChat(private_chat); User mock_user;
//   mock_user.id = user_id;
//   mock_user.email = fix.email;
//   mock_user.tag = fix.tag;
//   mock_user.name = fix.name;
//   mock_user.avatarPath = "avatar/test/path";

//   fix.presenter.setMockCurrentUser(mock_user);

//   SECTION("Send message with valid chat_id expected socket gets message to
//   send") {
//     QString message_to_send = "Hi, i'm from test";
//     int before = fix.fake_socket.sendTextMessage_calls;
//     fix.presenter.sendButtonClicked(message_to_send);

//     REQUIRE(fix.fake_socket.sendTextMessage_calls == before + 1);
//   }

//   SECTION("Send empty message expected message won't be send") {
//     QString message_to_send = "";
//     int before = fix.fake_socket.sendTextMessage_calls;
//     fix.presenter.sendButtonClicked(message_to_send);

//     REQUIRE(fix.fake_socket.sendTextMessage_calls == before);
//   }

//   User user;
//   user.id = 8;
//   auto private_chat4 = ChatFactory::createPrivateChat(chat_id, "r", "3", 8,
//   "2"); auto message_model = std::make_shared<MessageModel>();
//   fix.data_manager.addChat(private_chat4, message_model);
//   fix.data_manager.saveUser(user);
//   Message first_message;
//   first_message.id = 1;
//   first_message.text = "Roma";
//   first_message.chatId = chat_id;
//   first_message.local_id = "1local";
//   first_message.timestamp = QDateTime::currentDateTime();
//   message_model->saveMessage(first_message);

//   SECTION("Presenter receive new messages expected adding this message to
//   model") {
//     Message new_message;
//     new_message.id = 4;
//     new_message.text = "Roma";
//     new_message.chatId = 2;
//     new_message.senderId = 8;
//     new_message.local_id = "2local";
//     int before = message_model->rowCount();

//     fix.presenter.newMessage(new_message);

//     REQUIRE(message_model->rowCount() == before + 1);
//   }

//   SECTION("Presenter receive new messages expected this message will be last
//   message") {
//     int chat_id = 2;
//     Message new_message;
//     new_message.id = 4;
//     new_message.text = "Roma";
//     new_message.chatId = chat_id;
//     new_message.senderId = 8;
//     new_message.local_id = "2local";
//     new_message.timestamp = first_message.timestamp.addDays(1);
//     int before = message_model->rowCount();

//     fix.presenter.newMessage(new_message);

//     REQUIRE(message_model->rowCount() == before + 1);

//     auto returned_message = message_model->getLastMessage();
//     REQUIRE(returned_message != std::nullopt);
//     REQUIRE(returned_message->id == new_message.id);
//     REQUIRE(returned_message->text == new_message.text);
//     REQUIRE(returned_message->chatId == new_message.chatId);
//     REQUIRE(returned_message->senderId == new_message.senderId);
//   }

//   SECTION("Test presenter receive new messages before the oldest expected
//   this message will be oldest") {
//     Message new_message;
//     new_message.id = 4;
//     new_message.text = "Roma";
//     new_message.chatId = 2;
//     new_message.senderId = 8;
//     new_message.local_id = "2local";
//     new_message.timestamp = first_message.timestamp.addDays(-1);
//     int before = message_model->rowCount();

//     fix.presenter.newMessage(new_message);

//     REQUIRE(message_model->rowCount() == before + 1);

//     auto returned_message = message_model->getOldestMessage();
//     REQUIRE(returned_message != std::nullopt);
//     REQUIRE(returned_message->id == new_message.id);
//     REQUIRE(returned_message->text == new_message.text);
//     REQUIRE(returned_message->chatId == new_message.chatId);
//     REQUIRE(returned_message->senderId == new_message.senderId);
//   }

//   SECTION("Presenter receive responce socket connection opened expected send
//   initial socket message") {
//     QJsonObject json;
//     json["type"] = "opened";
//     int before_calls = fix.fake_socket.sendTextMessage_calls;

//     fix.presenter.onNewResponce(json);

//     REQUIRE(fix.fake_socket.sendTextMessage_calls == before_calls + 1);
//   }
// }

// TEST_CASE("Test sign in/up") {
//   TestPresenterFixrute fix;

//   SECTION("Presenter handle sig in expected network manager get to execute
//   POST method with valid params") {
//     LogInRequest login {
//       .email = fix.email,
//       .password = fix.password
//     };
//     int before_cnt_post = fix.netManager.post_counter;
//     std::string expected_url = "/auth/login";

//     fix.presenter.signIn(login);

//     REQUIRE(fix.netManager.post_counter == before_cnt_post + 1);
//     CHECK(fix.netManager.last_request.url().toString().toStdString() ==
//     expected_url); QJsonDocument doc =
//     QJsonDocument::fromJson(fix.netManager.last_data);
//     REQUIRE(doc.isObject());

//     QJsonObject body = doc.object();
//     CHECK(body["email"].toString() == login.email);
//     CHECK(body["password"].toString() == login.password);
//   }

//   SECTION("Presenter handle sig in expected network manager get to execute
//   POST method with valid params") {
//     SignUpRequest register_request {
//         .email = fix.email,
//         .password = fix.password,
//         .tag = fix.tag,
//         .name = fix.name
//     };
//     int before_cnt_post = fix.netManager.post_counter;
//     std::string expected_url = "/auth/register";

//     fix.presenter.signUp(register_request);

//     REQUIRE(fix.netManager.post_counter == before_cnt_post + 1);
//     CHECK(fix.netManager.last_request.url().toString().toStdString() ==
//     expected_url); QJsonDocument doc =
//     QJsonDocument::fromJson(fix.netManager.last_data);
//     REQUIRE(doc.isObject());

//     QJsonObject body = doc.object();
//     CHECK(body["email"].toString() == register_request.email);
//     CHECK(body["password"].toString() == register_request.password);
//     CHECK(body["tag"].toString() == register_request.tag);
//     CHECK(body["name"].toString() == register_request.name);
//   }
// }

// TEST_CASE("Test Presenter::onScroll") {
//   TestPresenterFixrute fix;
//   int chat_id = 1242;
//   int zero_value = 0;
//   fix.presenter.setMockOpenedChatId(chat_id);
//   fix.netManager.shouldFail = true;
//   auto reply_with_error = new MockReply();
//   reply_with_error->setMockError(QNetworkReply::ConnectionRefusedError,
//   "connection refused"); fix.netManager.setReply(reply_with_error);

//   SECTION("On scroll received non-zero value expected no call
//   getChatMessages") {
//     int value = 1;
//     int before_call_net_manager_ger = fix.netManager.get_counter;

//     fix.presenter.onScroll(value);

//     REQUIRE(fix.netManager.get_counter == before_call_net_manager_ger);
//   }

//   SECTION("On scroll received zero value but chat not opened expected no call
//   getChatMessages") {
//     fix.presenter.onLogOutButtonClicked();
//     int before_call_net_manager_ger = fix.netManager.get_counter;

//     fix.presenter.onScroll(zero_value);

//     REQUIRE(fix.netManager.get_counter == before_call_net_manager_ger);
//   }

//   SECTION("All valid expected call to getChatMessages") {
//     int before_call_net_manager_ger = fix.netManager.get_counter;

//     fix.presenter.onScroll(zero_value);

//     REQUIRE(fix.netManager.get_counter == before_call_net_manager_ger + 1);
//   }
// }
