#ifndef NEWMESSAGERESPONCEHANDLER_H
#define NEWMESSAGERESPONCEHANDLER_H

#include "JsonService.h"
#include "interfaces/ISocketResponceHandler.h"
#include "managers/TokenManager.h"
#include "managers/datamanager.h"

class NewMessageResponceHandler : public ISocketResponceHandler {
  EntityFactory *entity_factory_;
  DataManager *data_manager_;

 public:
  NewMessageResponceHandler(EntityFactory *entity_factory, DataManager *data_manager);

  void handle(const QJsonObject &json_object) override;
};

#endif  // NEWMESSAGERESPONCEHANDLER_H
