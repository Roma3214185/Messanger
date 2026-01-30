#ifndef OPENRESPONCEHANDLER_H
#define OPENRESPONCEHANDLER_H

#include <interfaces/ISocketResponceHandler.h>

#include "managers/TokenManager.h"
#include "usecases/socketusecase.h"

class OpenResponceHandler : public ISocketResponceHandler {
  TokenManager *token_manager_;
  SocketUseCase *socket_use_case_;

 public:
  OpenResponceHandler(TokenManager *token_manager, SocketUseCase *socket_use_case);

  void handle([[maybe_unused]] const QJsonObject &json_object) override;
};

#endif  // OPENRESPONCEHANDLER_H
