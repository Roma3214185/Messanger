#ifndef NEWMESSAHANDLER_H
#define NEWMESSAHANDLER_H

#include "interfaces/ISocketResponceHandler.h"

class IMessageDataManager;
class IReactionDataManager;
class IMessageJsonService;

class NewMessageHandler : public ISocketResponceHandler {
  IMessageJsonService *entity_factory_;
  IMessageDataManager *message_data_manager_;
  IReactionDataManager *reaction_data_manager_;

 public:
  NewMessageHandler(IMessageJsonService *entity_factory, IMessageDataManager *message_data_manager,
                    IReactionDataManager *reaction_data_manager);

  void handle(const QJsonObject &json_object) override;
};

#endif  // NEWMESSAHANDLER_H
