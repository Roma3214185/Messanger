#ifndef MARKREADMESSAGEHANDLER_H
#define MARKREADMESSAGEHANDLER_H

#include "interfaces/IMessageHandler.h"

class MarkReadMessageHandler : public IMessageHandler {
  public:
    void handle(const crow::json::rvalue& message,
                std::shared_ptr<ISocket>,
                NotificationManager& manager) override {

      if (!message.has("readed_by")) {
        LOG_ERROR("[mark_read] No readed_by");
        return;
      }

      auto msg = from_crow_json(message);
      int read_by = message["readed_by"].i();
      manager.onMarkReadMessage(msg, read_by);
      LOG_INFO("[mark_read] Message marked read by {}", read_by);
    }
};

#endif // MARKREADMESSAGEHANDLER_H
