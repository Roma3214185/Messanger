#ifndef SESSIONUSERCASE_H
#define SESSIONUSERCASE_H

#include <QObject>
#include <QString>

class SessionManager;
struct LogInRequest;
struct SignUpRequest;
struct User;

class ISessionUseCase : public QObject {
  Q_OBJECT
 public:
  virtual ~ISessionUseCase() = default;
  virtual void authentificatesWithToken(const QString &token) = 0;
  virtual void signIn(const LogInRequest &login_request) = 0;
  virtual void signUp(const SignUpRequest &signup_request) = 0;

 Q_SIGNALS:
  void userCreated(const User &, const QString &token);
};

class SessionUseCase : public ISessionUseCase {
  Q_OBJECT
 public:
  explicit SessionUseCase(std::unique_ptr<SessionManager> session_manager);
  void authentificatesWithToken(const QString &token) override;
  void signIn(const LogInRequest &login_request) override;
  void signUp(const SignUpRequest &signup_request) override;

 private:
  std::unique_ptr<SessionManager> session_manager_;
};

#endif  // SESSIONUSERCASE_H
