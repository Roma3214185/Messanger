#ifndef IUSECASEREPOSITORY_H
#define IUSECASEREPOSITORY_H

#include "chatusecase.h"
#include "messageusecase.h"
#include "sessionusecase.h"
#include "socketusecase.h"
#include "userusecase.h"

class IUseCaseRepository {
 public:
  virtual ~IUseCaseRepository() = default;

  virtual IChatUseCase* chat() = 0;
  virtual IMessageUseCase* message() = 0;
  virtual IUserUseCase* user() = 0;
  virtual ISessionUseCase* session() = 0;
  virtual ISocketUseCase* socket() = 0;
};

#endif  // IUSECASEREPOSITORY_H
