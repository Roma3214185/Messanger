#ifndef SESSIONUSERCASE_H
#define SESSIONUSERCASE_H

#include <QString>

class SessionManager;
class LogInRequest;
class SignUpRequest;

class SessionUseCase {
  public:
    SessionUseCase(SessionManager* session_manager);
    void authentificatesWithToken(const QString& token);
    void signIn(const LogInRequest& login_request);
    void signUp(const SignUpRequest& signup_request);
  private:
    SessionManager* session_manager_;
};

#endif // SESSIONUSERCASE_H
