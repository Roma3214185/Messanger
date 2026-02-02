#ifndef SOCKETHANDLERSREPOSITOTY_H
#define SOCKETHANDLERSREPOSITOTY_H

#include "interfaces/IMessageHandler.h"
#include "notificationservice/managers/NotificationOrchestrator.h"
#include "handlers/MessageHanldlers.h"

class ISocketHandlersRepository {
public:
    virtual ~ISocketHandlersRepository() = default;
    virtual void handle(const crow::json::rvalue &message, const std::shared_ptr<ISocket> &socket) = 0;
};

using SocketHandlers = std::unordered_map<std::string, std::unique_ptr<IMessageHandler>>;

class NotificationOrchestrator;

class SocketHandlersRepository : public ISocketHandlersRepository {
public:
    SocketHandlersRepository() {}

    void initHandlers(IPublisher* publisher, IUserSocketRepository* socket_repository) {
        handlers_["init"] = std::make_unique<InitMessageHandler>(socket_repository);
        handlers_["send_message"] = std::make_unique<SendMessageHandler>(publisher);
        handlers_["read_message"] = std::make_unique<MarkReadMessageHandler>(publisher);
        handlers_["save_reaction"] = std::make_unique<SaveMessageReactionHandler>(publisher);
        handlers_["delete_reaction"] = std::make_unique<DeleteMessageReactionHandler>(publisher);
    }


    void handle(const crow::json::rvalue &message, const std::shared_ptr<ISocket> &socket) override {
        if (!message.has("type")) {
          LOG_ERROR("[onMessage] No type");
          return;
        }

        const std::string &type = message["type"].s();
        if (auto it = handlers_.find(type); it != handlers_.end()) {
          LOG_INFO("Type is valid {}", type);
          it->second->handle(message, socket);
        } else {
          LOG_ERROR("Type isn't valid {}", type);
        }
    }


    SocketHandlers handlers_;
};

#endif // SOCKETHANDLERSREPOSITOTY_H
