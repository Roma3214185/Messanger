#ifndef OPENSOCKETHANDLER_H
#define OPENSOCKETHANDLER_H

#include <interfaces/ISocketResponceHandler.h>

class SocketUseCase;
class TokenManager;

class OpenSocketHandler : public ISocketResponceHandler {
 public:
  OpenSocketHandler(TokenManager *token_manager,
                       SocketUseCase *socket_use_case);
  void handle([[maybe_unused]] const QJsonObject &json_object) override;

 private:
  TokenManager *token_manager_;
  SocketUseCase *socket_use_case_;

};

#endif  // OPENSOCKETHANDLER_H
