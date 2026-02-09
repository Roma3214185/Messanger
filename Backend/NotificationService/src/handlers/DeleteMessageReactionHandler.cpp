#include "handlers/DeleteMessageReaction.h"

#include "entities/Reaction.h"
#include "notificationservice/IPublisher.h"

DeleteMessageReactionHandler::DeleteMessageReactionHandler(IPublisher *publisher) : publisher_(publisher) {}

void DeleteMessageReactionHandler::handle(const crow::json::rvalue &message,
                                          [[maybe_unused]] const std::shared_ptr<ISocket> &socket) {
  if (auto reaction = utils::entities::parseReaction(message); reaction.has_value()) {
    publisher_->deleteReaction(*reaction);
  }
}
