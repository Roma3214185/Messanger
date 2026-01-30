#include "handlers/DeleteMessageReaction.h"

void DeleteMessageReactionHandler::handle(const crow::json::rvalue &message,
            const std::shared_ptr<ISocket> &socket,
            NotificationManager &manager) {
    if (auto reaction = utils::entities::parseReaction(message); reaction.has_value()) {
        manager.deleteReaction(*reaction);
    }
}
