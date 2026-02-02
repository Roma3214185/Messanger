#ifndef DELETEMESSAGEREACTION_H
#define DELETEMESSAGEREACTION_H

#include "interfaces/IMessageHandler.h"

class IPublisher;

class DeleteMessageReactionHandler : public IMessageHandler {
 public:
  DeleteMessageReactionHandler(IPublisher* publisher);
  void handle(const crow::json::rvalue &message,
              const std::shared_ptr<ISocket> &socket) override;

  private:
  IPublisher* publisher_;
};

#endif  // DELETEMESSAGEREACTION_H
