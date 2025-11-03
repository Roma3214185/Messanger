#include <catch2/catch_all.hpp>

#include <QUrl>

#include "Managers/UserManager/usermanager.h"
#include "NetworkAccessManager/MockAccessManager.h"


TEST_CASE("Test user manager") {
  MockNetworkAccessManager network_manager;
  QUrl url("http://localhost:8083/");
  UserManager user_manager(&network_manager, url);
  int user_id = 4;
  auto reply = new MockReply();
  network_manager.setReply(reply);

  SECTION("GetUserExpectedRightUrlCreated") {
    QUrl resolved_url("http://localhost:8083/users/4");
    user_manager.getUser(user_id);
    REQUIRE(network_manager.last_request.url() == resolved_url);
  }

  SECTION("GetUserExpectedGetMethod") {
    int before_get_calls = network_manager.get_counter;
    user_manager.getUser(user_id);
    REQUIRE(network_manager.get_counter == before_get_calls + 1);
  }
}








// auto UserManager::getUser(int user_id) -> optional<User> {

//   QUrl endpoint = url_.resolved(QUrl(QString("/users/%1").arg(user_id)));
//   QNetworkRequest req(endpoint);
//   auto* reply = network_manager_->get(req);

//   QEventLoop loop;
//   QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
//   loop.exec();

//   return onGetUser(reply);
// }

// auto UserManager::onGetUser(QNetworkReply* reply) -> optional<User> {
//   PROFILE_SCOPE("UserManager::onGetUser");
//   QScopedPointer<QNetworkReply, QScopedPointerDeleteLater>
//       guard(reply);

//   if (reply->error() != QNetworkReply::NoError) {
//     LOG_ERROR("[onGetUser] Network error: '{}'",
//               reply->errorString().toStdString());
//     //Q_EMIT errorOccurred("get user: " + reply->errorString());
//     return std::nullopt;
//   }

//   auto doc = QJsonDocument::fromJson(reply->readAll());
//   if (!doc.isObject()) {
//     LOG_ERROR("[onGetUser] Invalid JSON: expected object at root");
//     //Q_EMIT errorOccurred("Invalid JSON: expected object at root");
//     return std::nullopt;
//   }

//   auto user = JsonService::getUserFromResponse(doc.object());
//   LOG_INFO("[onGetUser] User loaded: '{}'", user.name.toStdString());
//   return user;
// }

// QList<User> UserManager::findUsersByTag(const QString& tag) {
//   QUrl endpoint =
//       url_.resolved(QUrl(QString("/users/search?tag=%1").arg(tag)));
//   auto request = QNetworkRequest(endpoint);
//   auto* reply = network_manager_->get(request);
//   QEventLoop loop;
//   QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
//   loop.exec();
//   auto users = findUsersByTag(reply);
//   return users;
// }

// QList<User> UserManager::findUsersByTag(QNetworkReply* reply) {
//   QScopedPointer<QNetworkReply, QScopedPointerDeleteLater> guard(reply);
//   if (reply->error() != QNetworkReply::NoError) {
//     //Q_EMIT errorOccurred("onFindUsers" + reply->errorString());
//     return {};
//   }
//   auto responseData = reply->readAll();
//   auto doc = QJsonDocument::fromJson(responseData);
//   if (!doc.isObject()) {
//     //Q_EMIT errorOccurred("Invalid JSON: expected object at root");
//     return {};
//   }
//   auto rootObj = doc.object();
//   auto arr = rootObj["users"].toArray();
//   QList<User> users;
//   for (const auto& value : std::as_const(arr)) {
//     auto obj = value.toObject();
//     auto user = JsonService::getUserFromResponse(obj);
//     users.append(user);
//   }
//   return users;
// }
