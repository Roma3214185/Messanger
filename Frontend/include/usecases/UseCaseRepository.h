#ifndef USECASEREPOSITORY_H
#define USECASEREPOSITORY_H

#include "chatusecase.h"
#include "messageusecase.h"
#include "sessionusecase.h"
#include "socketusecase.h"
#include "userusecase.h"

class UseCaseRepository {
 public:
  UseCaseRepository(std::unique_ptr<IChatUseCase> chat, std::unique_ptr<IMessageUseCase> message,
                    std::unique_ptr<IUserUseCase> user, std::unique_ptr<ISessionUseCase> session,
                    std::unique_ptr<ISocketUseCase> socket);

  IChatUseCase* chat();
  IMessageUseCase* message();
  IUserUseCase* user();
  ISessionUseCase* session();
  ISocketUseCase* socket();

 private:
  std::unique_ptr<IChatUseCase> chat_;
  std::unique_ptr<IMessageUseCase> message_;
  std::unique_ptr<IUserUseCase> user_;
  std::unique_ptr<ISessionUseCase> session_;
  std::unique_ptr<ISocketUseCase> socket_;
};

#endif  // USECASEREPOSITORY_H
