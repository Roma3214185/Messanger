#ifndef OPENSOCKETHANDLER_H
#define OPENSOCKETHANDLER_H

#include <interfaces/ISocketResponceHandler.h>

class ISocketUseCase;
class TokenManager;

class OpenSocketHandler : public ISocketResponceHandler {
  TokenManager *token_manager_;
  ISocketUseCase *socket_use_case_;

 public:
  OpenSocketHandler(TokenManager *token_manager, ISocketUseCase *socket_use_case);

  void handle([[maybe_unused]] const QJsonObject &json_object) override;
};

#endif  // OPENSOCKETHANDLER_H
