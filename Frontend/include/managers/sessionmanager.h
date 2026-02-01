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
class IUserJsonService;

class SessionManager : public BaseManager {
  Q_OBJECT
  IUserJsonService *entity_factory_;

 public:
  SessionManager(IUserJsonService *, INetworkAccessManager *network_manager, const QUrl &base_url,
                 std::chrono::milliseconds timeout_ms = std::chrono::milliseconds{500}, QObject *parent = nullptr);

  void signIn(const LogInRequest &login_request);
  void signUp(const SignUpRequest &signup_request);
  void authenticateWithToken(const QString &token);

 protected:
  virtual void onReplyFinished(const QByteArray &responce);

 Q_SIGNALS:
  void userCreated(const User &user, const QString &token);
};

#endif  // SESSIONMANAGER_H
