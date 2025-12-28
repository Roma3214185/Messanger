#include "usecases/sessionusecase.h"

#include "managers/sessionmanager.h"

SessionUseCase::SessionUseCase(SessionManager* session_manager)
    : session_manager_(session_manager) {
  connect(session_manager_, &SessionManager::userCreated, this, &SessionUseCase::userCreated);
}

void SessionUseCase::authentificatesWithToken(const QString& token ) {
  DBC_REQUIRE(!token.isEmpty());
  session_manager_->authenticateWithToken(token);
}

void SessionUseCase::signIn(const LogInRequest& login_request) {
  session_manager_->signIn(login_request);
}

void SessionUseCase::signUp(const SignUpRequest& signup_request) {
  session_manager_->signUp(signup_request);
}
