#include "handlers/SendMessageHandler.h"
#include "Debug_profiling.h"
#include "entities/Message.h"
#include "notificationservice/IPublisher.h"

SendMessageHandler::SendMessageHandler(IPublisher* publisher) : publisher_(publisher) {}

void SendMessageHandler::handle(const crow::json::rvalue &message, const std::shared_ptr<ISocket> &socket) {
  auto msg = utils::entities::from_crow_json(message);
  publisher_->saveMessage(msg);
  LOG_INFO("[send_message] Message processed");
}
