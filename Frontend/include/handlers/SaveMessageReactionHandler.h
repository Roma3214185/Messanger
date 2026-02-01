#ifndef SAVEMESSAGEREACTIONHANDLER_H
#define SAVEMESSAGEREACTIONHANDLER_H

#include "interfaces/ISocketResponceHandler.h"

class IReactionDataManager;
class IReactionJsonService;

class SaveMessageReactionHandler : public ISocketResponceHandler {
  IReactionJsonService *entity_factory_;
  IReactionDataManager *data_manager_;

 public:
  SaveMessageReactionHandler(IReactionJsonService *entity_factory, IReactionDataManager *data_manager);

  void handle(const QJsonObject &json_object) override;
};

#endif  // SAVEMESSAGEREACTIONHANDLER_H
