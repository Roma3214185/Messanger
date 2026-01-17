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
  NewMessageResponceHandler(EntityFactory *entity_factory, DataManager *data_manager)
      : entity_factory_(entity_factory), data_manager_(data_manager) {}

  void handle(const QJsonObject &json_object) override {
    auto [message, reactions] = entity_factory_->getMessageFromJson(json_object);
    data_manager_->save(message);
    data_manager_->save(reactions);
  }
};

#endif  // NEWMESSAGERESPONCEHANDLER_H
