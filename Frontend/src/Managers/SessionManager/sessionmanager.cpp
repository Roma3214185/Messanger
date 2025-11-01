#include "sessionmanager.h"

#include <QNetworkReply>
#include <QString>
#include <QJsonObject>
#include <QNetworkRequest>
#include <QJsonDocument>

#include "headers/INetworkAccessManager.h"
#include "DebugProfiling/Debug_profiling.h"
#include "headers/JsonService.h"
#include "headers/User.h"
#include "headers/SignUpRequest.h"

SessionManager::SessionManager(INetworkAccessManager* net_manager, QUrl url)
    : net_manager_(net_manager), url_(url) {}

void SessionManager::signIn(const LogInRequest& login_request){
  PROFILE_SCOPE("SessionManager::signIn");
  LOG_INFO("[signIn] Attempting login for email '{}'",
           login_request.email.toStdString());

  QUrl endpoint = url_.resolved(QUrl("/auth/login"));
  QNetworkRequest req(endpoint);
  req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
  LOG_INFO("Url setted");

  QJsonObject body{{"email", login_request.email},
                   {"password", login_request.password}};
  LOG_INFO("Json created");
  auto temp = QJsonDocument(body).toJson();
  LOG_INFO("temp created");
  if(!net_manager_) LOG_INFO("nullptr");
  else LOG_INFO(" not nullptr");
  auto reply = net_manager_->post(req, temp);
  //auto reply = net_manager_->post(req, QJsonDocument(body).toJson());
  LOG_INFO("reply sended");

  QObject::connect(reply, &QNetworkReply::finished, this,
                   [this, reply]() -> void { onSignInFinished(reply); });
}

void SessionManager::onSignInFinished(QNetworkReply* reply) {
  PROFILE_SCOPE("SessionManager::onSignInFinished");
  QScopedPointer<QNetworkReply, QScopedPointerDeleteLater> guard(reply);

  if (reply->error() != QNetworkReply::NoError) {
    LOG_ERROR("[onSignInFinished] Network error: '{}'",
              reply->errorString().toStdString());
    Q_EMIT errorOccurred(reply->errorString());
    return;
  }

  auto jsonResponse = QJsonDocument::fromJson(reply->readAll());
  auto responseObj = jsonResponse.object();
  auto createdUser =
      JsonService::getUserFromResponse(responseObj["user"].toObject());
  QString current_token = responseObj["token"].toString();

  LOG_INFO("[onSignInFinished] Login success. User: '{}', Token: '{}'",
           createdUser.name.toStdString(), current_token.toStdString());
  Q_EMIT userCreated(createdUser, current_token);
}

void SessionManager::onSignUpFinished(QNetworkReply *reply){
  PROFILE_SCOPE("Model::onSignUpFinished");
  QScopedPointer<QNetworkReply, QScopedPointerDeleteLater> guard(reply);

  if (reply->error() != QNetworkReply::NoError) {
    LOG_ERROR("[onSignUpFinished] Network error: '{}'",
              reply->errorString().toStdString());
    Q_EMIT errorOccurred(reply->errorString());
    return;
  }

  auto jsonResponse = QJsonDocument::fromJson(reply->readAll());
  auto responseObj = jsonResponse.object();
  auto createdUser =
      JsonService::getUserFromResponse(responseObj["user"].toObject());
  QString current_token = responseObj["token"].toString();

  LOG_INFO("[onSignUpFinished] Registration success. User: '{}', Token: '{}'",
           createdUser.name.toStdString(), current_token.toStdString());
  Q_EMIT userCreated(createdUser, current_token);
}

void SessionManager::onAuthenticate(QNetworkReply* reply) {
  PROFILE_SCOPE("Model::onAuthenticate");
  QScopedPointer<QNetworkReply, QScopedPointerDeleteLater> guard(reply);

  if (reply->error() != QNetworkReply::NoError) {
    spdlog::warn("OnAuthenticate failed: '{}'", reply->errorString().toStdString());
    return;
  }

  auto response_data = reply->readAll();
  QJsonParseError parse_error;
  auto json_response = QJsonDocument::fromJson(response_data, &parse_error);
  if (parse_error.error != QJsonParseError::NoError ||
      !json_response.isObject()) {
    LOG_ERROR("Sign me failed: invalid JSON - '{}'",
              parse_error.errorString().toStdString());
    return;
  }

  auto response_obj = json_response.object();

  if (!response_obj.contains("user") || !response_obj["user"].isObject()) {
    LOG_ERROR("Sign me failed: JSON does not contain 'user' object");
    return;
  }
  QString current_token;

  auto created_user =
      JsonService::getUserFromResponse(response_obj["user"].toObject());

  if (!response_obj.contains("token") || !response_obj["token"].isString()) {
    spdlog::warn("Sign me succeeded but no token returned for user '{}'",
                 created_user.name.toStdString());
    current_token.clear();
  } else {
    current_token = response_obj["token"].toString();
    LOG_INFO("Sign me success: user '{}' with token '{}'",
             created_user.name.toStdString(), current_token.toStdString());
  }

  Q_EMIT userCreated(created_user, current_token);
}

void SessionManager::signUp(const SignUpRequest& signup_request){
  PROFILE_SCOPE("Model::signUp");
  LOG_INFO("[signUp] Registering new user: '{}'", signup_request.email.toStdString());

  QUrl endpoint = url_.resolved(QUrl("/auth/register"));
  QNetworkRequest req(endpoint);
  req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

  QJsonObject body{{"email", signup_request.email},
                   {"password", signup_request.password},
                   {"name", signup_request.name},
                   {"tag", signup_request.tag}};
  auto reply = net_manager_->post(req, QJsonDocument(body).toJson());

  connect(reply, &QNetworkReply::finished, this,
          [this, reply]() -> void { onSignUpFinished(reply); });
}

void SessionManager::authenticateWithToken(const QString& token) {
  QUrl url("http://localhost:8083");
  QUrl endpoint = url.resolved(QUrl("/auth/me"));
  auto request = QNetworkRequest(endpoint);
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
  request.setRawHeader("Authorization", token.toUtf8());
  auto* reply = net_manager_->get(request);
  QObject::connect(reply, &QNetworkReply::finished, this,
                   [this, reply]() -> void { onAuthenticate(reply); });
}



