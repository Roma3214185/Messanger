#ifndef DELETEMESSAGEREACTIONHANDLER_H
#define DELETEMESSAGEREACTIONHANDLER_H

#include <QJsonObject>
#include "interfaces/ISocketResponceHandler.h"
#include "managers/datamanager.h"

class DeleteMessageReactionHandler : public ISocketResponceHandler {
  EntityFactory *entity_factory_;
  DataManager *data_manager_;

 public:
  DeleteMessageReactionHandler(EntityFactory *entity_factory, DataManager *data_manager);
  void handle(const QJsonObject &json_object) override;
};

#endif  // DELETEMESSAGEREACTIONHANDLER_H
