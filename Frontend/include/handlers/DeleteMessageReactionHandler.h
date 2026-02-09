#ifndef DELETEMESSAGEREACTIONHANDLER_H
#define DELETEMESSAGEREACTIONHANDLER_H

#include "interfaces/ISocketResponceHandler.h"

class IReactionJsonService;
class IReactionDataManager;

class DeleteMessageReactionHandler : public ISocketResponceHandler {
 public:
  DeleteMessageReactionHandler(IReactionJsonService *entity_factory,
                                  IReactionDataManager *data_manager);
  void handle(const QJsonObject &json_object) override;

 private:
  IReactionJsonService *entity_factory_;
  IReactionDataManager *data_manager_;

};

#endif  // DELETEMESSAGEREACTIONHANDLER_H
