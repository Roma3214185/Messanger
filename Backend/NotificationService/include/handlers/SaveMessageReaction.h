#ifndef SAVEMESSAGEREACTION_H
#define SAVEMESSAGEREACTION_H

#include "interfaces/IMessageHandler.h"

class IPublisher;

class SaveMessageReactionHandler : public IMessageHandler {
 public:
  SaveMessageReactionHandler(IPublisher* publisher);
  void handle(const crow::json::rvalue &message, const std::shared_ptr<ISocket> &socket);

private:
  IPublisher* publisher_;
};

#endif  // SAVEMESSAGEREACTION_H
