#ifndef DEFAULTUSECASEREPOSITORY_H
#define DEFAULTUSECASEREPOSITORY_H

#include "IUseCaseRepository.h"

class DefaultUseCaseRepository : public IUseCaseRepository {
 public:
  DefaultUseCaseRepository(std::unique_ptr<IChatUseCase> chat, std::unique_ptr<IMessageUseCase> message,
                           std::unique_ptr<IUserUseCase> user, std::unique_ptr<ISessionUseCase> session,
                           std::unique_ptr<ISocketUseCase> socket);

  IChatUseCase* chat() override;
  IMessageUseCase* message() override;
  IUserUseCase* user() override;
  ISessionUseCase* session() override;
  ISocketUseCase* socket() override;

 private:
  std::unique_ptr<IChatUseCase> chat_;
  std::unique_ptr<IMessageUseCase> message_;
  std::unique_ptr<IUserUseCase> user_;
  std::unique_ptr<ISessionUseCase> session_;
  std::unique_ptr<ISocketUseCase> socket_;
};

#endif  // DEFAULTUSECASEREPOSITORY_H
