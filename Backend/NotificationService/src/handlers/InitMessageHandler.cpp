#include "handlers/InitMessageHandler.h"
#include "Debug_profiling.h"
#include "notificationservice/SocketRepository.h"
#include "entities/Reaction.h" //todo: move getFiels to another class

InitMessageHandler::InitMessageHandler(IUserSocketRepository *socket_repository)
    : socket_repository_(socket_repository) {}

void InitMessageHandler::handle(const crow::json::rvalue &message, const std::shared_ptr<ISocket> &socket) {
  if(!message.has("user_id")) {
    LOG_ERROR("No user_id");
    return;
  }

  if(!socket) {
      LOG_ERROR("Invalid socket");
      return;
  }

  const long long user_id = message["user_id"].i();
  if (user_id <= 0) {
      LOG_ERROR("Invalid user_id value: {}", user_id);
      return;
  }

  socket_repository_->saveConnections(user_id, socket);
  LOG_INFO("Socket registered for userId '{}'", user_id);
}
