#include "handlers/SaveMessageReaction.h"

void SaveMessageReactionHandler::handle(const crow::json::rvalue &message, const std::shared_ptr<ISocket> &socket,
                                        NotificationManager &manager) {
  if (auto reaction = utils::entities::parseReaction(message);
      reaction.has_value()) {  // todo: u can here fully implement SaveMessageReaction in mq
    manager.saveReaction(*reaction);
  }
}
