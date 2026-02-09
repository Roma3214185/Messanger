#ifndef SOCKETHANDLERREGISTRY_H
#define SOCKETHANDLERREGISTRY_H

#include <unordered_map>

class ISocketResponceHandler;
class JsonService;
class Model;

class SocketHandlerRegistry {
 public:
  using SocketHandlersMap = std::unordered_map<std::string, std::unique_ptr<ISocketResponceHandler>>;
  static SocketHandlersMap create(Model* manager, JsonService* json_service);
};

#endif  // SOCKETHANDLERREGISTRY_H
