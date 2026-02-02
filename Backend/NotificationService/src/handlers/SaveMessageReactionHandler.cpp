#include "handlers/SaveMessageReaction.h"
#include "notificationservice/IPublisher.h"
#include "entities/Reaction.h"

SaveMessageReactionHandler::SaveMessageReactionHandler(IPublisher *publisher) : publisher_(publisher) {}

void SaveMessageReactionHandler::handle(const crow::json::rvalue &message, const std::shared_ptr<ISocket> &socket) {
  if (auto reaction = utils::entities::parseReaction(message);
      reaction.has_value()) {
    publisher_->saveReaction(*reaction);
  }
}
