#include "managers/sessionmanager.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QString>

#include "Debug_profiling.h"
#include "JsonService.h"
#include "dto/SignUpRequest.h"
#include "dto/User.h"
#include "interfaces/INetworkAccessManager.h"

namespace {

auto getRequestWithToken(QUrl endpoint, const QString& current_token) -> QNetworkRequest {
  auto request = QNetworkRequest(endpoint);
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
  request.setRawHeader("Authorization", current_token.toUtf8());
  return request;
}

}  // namespace

void SessionManager::signIn(const LogInRequest& login_request) {
  PROFILE_SCOPE("SessionManager::signIn");
  LOG_INFO("[signIn] Attempting login for email '{}'", login_request.email.toStdString());

  QUrl            endpoint = url_.resolved(QUrl("/auth/login"));
  QNetworkRequest req(endpoint);
  req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

  QJsonObject body{{"email", login_request.email}, {"password", login_request.password}};
  auto        reply = network_manager_->post(req, QJsonDocument(body).toJson());

  handleReplyWithTimeoutVoid(
      reply,
      [this](const QByteArray& responce_data) { return onReplyFinished(responce_data); },
      timeout_ms_);
}

void SessionManager::onReplyFinished(const QByteArray& responce) {
  PROFILE_SCOPE("SessionManager::onReplyFinished");
  // if(!checkReply(reply)) return;
  // QScopedPointer<QNetworkReply, QScopedPointerDeleteLater> guard(reply);
  LOG_INFO("Raw responce: {}", responce.toStdString());
  auto jsonResponse = QJsonDocument::fromJson(responce);
  auto responseObj  = jsonResponse.object();
  if (!responseObj.contains("user")) {  // TODO: common checkField(responseObj, "user") function;
    LOG_ERROR("Reply doen't contain 'user' filed");
    return;
  }

  if (!responseObj.contains("token")) {
    LOG_ERROR("Reply doen't contain 'token' filed");
    return;
  }

  auto    createdUser   = JsonService::getUserFromResponse(responseObj["user"].toObject());
  QString current_token = responseObj["token"].toString();

  LOG_INFO("[onReplyFinished] User created. User: '{}', Token: '{}'",
           createdUser.name.toStdString(),
           current_token.toStdString());  // TODO: debug(createdUser);
  Q_EMIT userCreated(createdUser, current_token);
}

void SessionManager::signUp(const SignUpRequest& signup_request) {
  PROFILE_SCOPE("Model::signUp");
  LOG_INFO("[signUp] Registering new user: '{}'", signup_request.email.toStdString());

  QUrl            endpoint = url_.resolved(QUrl("/auth/register"));
  QNetworkRequest req(endpoint);
  req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

  QJsonObject body{{"email", signup_request.email},
                   {"password", signup_request.password},
                   {"name", signup_request.name},
                   {"tag", signup_request.tag}};
  auto*       reply = network_manager_->post(req, QJsonDocument(body).toJson());

  handleReplyWithTimeoutVoid(
      reply,
      [this](const QByteArray& responce_data) { return onReplyFinished(responce_data); },
      timeout_ms_);
}

void SessionManager::authenticateWithToken(const QString& token) {
  QUrl  endpoint = url_.resolved(QUrl("/auth/me"));
  auto  req      = getRequestWithToken(endpoint, token);
  auto* reply    = network_manager_->get(req);
  handleReplyWithTimeoutVoid(
      reply,
      [this](const QByteArray& responce_data) { return onReplyFinished(responce_data); },
      timeout_ms_);
}
