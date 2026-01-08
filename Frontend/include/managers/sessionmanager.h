#ifndef SESSIONMANAGER_H
#define SESSIONMANAGER_H

#include <QObject>
#include <QUrl>

#include "managers/BaseManager.h"

class INetworkAccessManager;
struct LogInRequest;
class QNetworkReply;
struct User;
struct SignUpRequest;

class SessionManager : public BaseManager {
  Q_OBJECT
 public:
  using BaseManager::BaseManager;

  void signIn(const LogInRequest &login_request);
  void signUp(const SignUpRequest &signup_request);
  void authenticateWithToken(const QString &token);

 protected:
  virtual void onReplyFinished(const QByteArray &responce);

 Q_SIGNALS:
  void userCreated(const User &user, const QString &token);
};

#endif  // SESSIONMANAGER_H
