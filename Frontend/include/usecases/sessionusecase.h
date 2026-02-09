#ifndef SESSIONUSERCASE_H
#define SESSIONUSERCASE_H

#include <QObject>
#include <QString>

#include "managers/sessionmanager.h"

struct LogInRequest;
struct SignUpRequest;
struct User;

class SessionUseCase : public QObject {
  Q_OBJECT
 public:
  explicit SessionUseCase(std::unique_ptr<SessionManager> session_manager);
  void authentificatesWithToken(const QString &token);
  void signIn(const LogInRequest &login_request);
  void signUp(const SignUpRequest &signup_request);

 Q_SIGNALS:
  void userCreated(const User &, const QString &token);

 private:
  std::unique_ptr<SessionManager> session_manager_;
};

#endif  // SESSIONUSERCASE_H
