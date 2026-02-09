#include "SocketHandlersRepositoty.h"

void SocketHandlersRepository::setHandlers(SocketHandlers &&handlers) { handlers_ = std::move(handlers); }

void SocketHandlersRepository::handle(const crow::json::rvalue &message, const std::shared_ptr<ISocket> &socket) {
  if (!message.has("type")) {
    LOG_ERROR("[onMessage] No type");
    return;
  }

  const std::string &type = message["type"].s();
  if (auto it = handlers_.find(type); it != handlers_.end()) {
    LOG_INFO("Type is valid {}", type);
    it->second->handle(message, socket);
  } else {
    LOG_ERROR("Type isn't valid {}", type);
  }
}
