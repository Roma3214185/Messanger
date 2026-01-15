#ifndef SAVEMESSAGEREACTIONHANDLER_H
#define SAVEMESSAGEREACTIONHANDLER_H

#include "JsonService.h"
#include "interfaces/ISocketResponceHandler.h"
#include "usecases/messageusecase.h"

class SaveMessageReactionHandler : public ISocketResponceHandler {
  EntityFactory *entity_factory_;
  DataManager *data_manager_;

 public:
  SaveMessageReactionHandler(EntityFactory *entity_factory, DataManager *data_manager)
      : entity_factory_(entity_factory), data_manager_(data_manager) {}

  void handle(const QJsonObject &json_object) override {
    auto reaction = entity_factory_->getReaction(json_object);
    data_manager_->saveReaction(reaction);
  }
};

#endif  // SAVEMESSAGEREACTIONHANDLER_H
