#ifndef INITMESSAGEHANDLER_H
#define INITMESSAGEHANDLER_H

#include "interfaces/IMessageHandler.h"

class InitMessageHandler : public IMessageHandler {
  public:
    void handle(const crow::json::rvalue& message,
                const std::shared_ptr<ISocket>& socket,
                NotificationManager& manager) override {
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
          return static_cast<long long>(message["user_id"].d());
        }

        LOG_ERROR("Unexpected type of field 'user_id'");
        return std::nullopt;

      }();

      if(user_id) {
        manager.userConnected(*user_id, socket);
        LOG_INFO("[init] Socket registered for userId '{}'", *user_id);
      }
    }
};

#endif // INITMESSAGEHANDLER_H
