#include "handlers/InitMessageHandler.h"
#include "Debug_profiling.h"
#include "notificationservice/SocketRepository.h"

InitMessageHandler::InitMessageHandler(IUserSocketRepository *socket_repository)
    : socket_repository_(socket_repository) {}

void InitMessageHandler::handle(const crow::json::rvalue &message, const std::shared_ptr<ISocket> &socket) {
  LOG_INFO("Try get user_id");
  auto user_id = [message]() -> std::optional<long long> {
    if (!message.has("user_id")) {
      LOG_ERROR("[init] No user_id");
      return std::nullopt;
    }

    if (message["user_id"].t() == crow::json::type::String) {
      return std::stoll(message["user_id"].s());
    }

    if (message["user_id"].t() == crow::json::type::Number) {
      return static_cast<long long>(message["user_id"].i());
    }

    LOG_ERROR("Unexpected type of field 'user_id'");
    return std::nullopt;
  }();

  if (user_id.has_value()) {
    socket_repository_->saveConnections(*user_id, socket);
    LOG_INFO("[init] Socket registered for userId '{}'", *user_id);
  } else {
    LOG_ERROR("Invalid user_id");
  }
}
