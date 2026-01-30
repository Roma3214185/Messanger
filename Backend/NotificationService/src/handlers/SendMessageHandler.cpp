#include "handlers/SendMessageHandler.h"

void SendMessageHandler::handle(const crow::json::rvalue &message, const std::shared_ptr<ISocket> &socket,
                                NotificationManager &manager) {
  auto msg = utils::entities::from_crow_json(message);  // todo utils::parsePayload<Message>(message.dump())
  manager.onSendMessage(msg);
  LOG_INFO("[send_message] Message processed");
}
