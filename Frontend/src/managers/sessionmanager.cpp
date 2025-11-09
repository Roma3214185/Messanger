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
      [this](QNetworkReply* server_reply) { return onReplyFinished(server_reply); },
      timeout_ms_);
}

void SessionManager::onReplyFinished(QNetworkReply* reply) {
  PROFILE_SCOPE("SessionManager::onReplyFinished");
  QScopedPointer<QNetworkReply, QScopedPointerDeleteLater> guard(reply);

  if (reply->error() != QNetworkReply::NoError) {
    LOG_ERROR("[onReplyFinished] Network error: '{}'", reply->errorString().toStdString());
    Q_EMIT errorOccurred(reply->errorString());
    return;
  }

  auto    jsonResponse  = QJsonDocument::fromJson(reply->readAll());
  auto    responseObj   = jsonResponse.object();
  auto    createdUser   = JsonService::getUserFromResponse(responseObj["user"].toObject());
  QString current_token = responseObj["token"].toString();

  LOG_INFO("[onReplyFinished] User created. User: '{}', Token: '{}'",
           createdUser.name.toStdString(),
           current_token.toStdString());
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
      [this](QNetworkReply* server_reply) { return onReplyFinished(server_reply); },
      timeout_ms_);
}

void SessionManager::authenticateWithToken(const QString& token) {
  QUrl endpoint = url_.resolved(QUrl("/auth/me"));
  auto request  = QNetworkRequest(endpoint);
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
  request.setRawHeader("Authorization", token.toUtf8());
  auto* reply = network_manager_->get(request);
  handleReplyWithTimeoutVoid(
      reply,
      [this](QNetworkReply* server_reply) { return onReplyFinished(server_reply); },
      timeout_ms_);
}
