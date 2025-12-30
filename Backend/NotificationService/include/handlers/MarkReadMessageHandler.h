#ifndef MARKREADMESSAGEHANDLER_H
#define MARKREADMESSAGEHANDLER_H

#include "interfaces/IMessageHandler.h"

class MarkReadMessageHandler : public IMessageHandler {
  public:
    void handle(const crow::json::rvalue& message,
                const std::shared_ptr<ISocket>& socket,
                NotificationManager& manager) override {

      LOG_INFO("Try get user_id");
      auto read_by = [message]() -> std::optional<long long> {
        if (!message.has("readed_by")) {
          LOG_ERROR(" No readed_by");
          return std::nullopt;
        }

        if (message["readed_by"].t() == crow::json::type::String) {
          return std::stoll(message["readed_by"].s());
        }

        if (message["readed_by"].t() == crow::json::type::Number) {
          return static_cast<long long>(message["readed_by"].d());
        }

        LOG_ERROR("Unexpected type of field 'readed_by'");
        return std::nullopt;
      }();

      if(!read_by) return;

      auto msg = utils::entities::from_crow_json(message);
      manager.onMarkReadMessage(msg, *read_by);
      LOG_INFO("[mark_read] Message marked read by {}", *read_by);
    }
};

#endif // MARKREADMESSAGEHANDLER_H
