#ifndef MARKREADMESSAGEHANDLER_H
#define MARKREADMESSAGEHANDLER_H

#include "interfaces/IMessageHandler.h"

class MarkReadMessageHandler : public IMessageHandler {
  public:
    void handle(const crow::json::rvalue& message,
                const std::shared_ptr<ISocket>& socket,
                NotificationManager& manager) override {
      long long read_by = static_cast<long long>(message["readed_by"].d());
      long long message_id = static_cast<long long>(message["message_id"].d());
      MessageStatus message_status;
      //todo: check if this user is member of chat in message service
      message_status.is_read = true;
      message_status.message_id = message_id;
      message_status.receiver_id = read_by;
      message_status.read_at = getCurrentTime();
      manager.saveMessageStatus(message_status);
      LOG_INFO("[mark_read] Message marked read {}", nlohmann::json(message_status).dump());
    }
};

//todo: now will be bug: 2 times saved read_message, offline and online, i think i need skip if my_id == read_by

#endif // MARKREADMESSAGEHANDLER_H
