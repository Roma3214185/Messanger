#include "handlers/MarkReadMessageHandler.h"
#include "entities/MessageStatus.h"
#include "notificationservice/IPublisher.h"

MarkReadMessageHandler::MarkReadMessageHandler(IPublisher *publisher) : publisher_(publisher) {}

void MarkReadMessageHandler::handle(const crow::json::rvalue &message, const std::shared_ptr<ISocket> &socket) {
    if(!message.has("readed_by")) {
        LOG_ERROR("There is no readed_by field");
        return;
    }

    if(!message.has("message_id")) {
        LOG_ERROR("There is no message_id field");
        return;
    }


  const auto read_by = static_cast<long long>(message["readed_by"].i());
  const auto message_id = static_cast<long long>(message["message_id"].i());

  MessageStatus message_status;
  // todo: check if this user is member of chat in message service
  message_status.is_read = true;
  message_status.message_id = message_id;
  message_status.receiver_id = read_by;
  message_status.read_at = utils::time::getCurrentTime();
  publisher_->saveMessageStatus(message_status);
  LOG_INFO("[mark_read] Message marked read {}", nlohmann::json(message_status).dump());
}
