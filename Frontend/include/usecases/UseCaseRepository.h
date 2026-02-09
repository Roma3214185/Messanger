#ifndef USECASEREPOSITORY_H
#define USECASEREPOSITORY_H

#include "chatusecase.h"
#include "messageusecase.h"
#include "sessionusecase.h"
#include "socketusecase.h"
#include "userusecase.h"

class UseCaseRepository {
 public:
  UseCaseRepository(std::unique_ptr<ChatUseCase> chat, std::unique_ptr<MessageUseCase> message,
                    std::unique_ptr<UserUseCase> user, std::unique_ptr<SessionUseCase> session,
                    std::unique_ptr<SocketUseCase> socket);

  ChatUseCase* chat();
  MessageUseCase* message();
  UserUseCase* user();
  SessionUseCase* session();
  SocketUseCase* socket();

 private:
  std::unique_ptr<ChatUseCase> chat_;
  std::unique_ptr<MessageUseCase> message_;
  std::unique_ptr<UserUseCase> user_;
  std::unique_ptr<SessionUseCase> session_;
  std::unique_ptr<SocketUseCase> socket_;
};

#endif  // USECASEREPOSITORY_H
