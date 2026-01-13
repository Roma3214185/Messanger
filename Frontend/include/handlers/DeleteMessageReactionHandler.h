#ifndef DELETEMESSAGEREACTIONHANDLER_H
#define DELETEMESSAGEREACTIONHANDLER_H

#include <QJsonObject>
#include "interfaces/ISocketResponceHandler.h"
#include "managers/datamanager.h"

class DeleteMessageReactionHandler : public ISocketResponceHandler {
  EntityFactory *entity_factory_;
  DataManager *data_manager_;

 public:
  DeleteMessageReactionHandler(EntityFactory *entity_factory, DataManager *data_manager)
      : entity_factory_(entity_factory), data_manager_(data_manager) {}

  void handle(const QJsonObject &json_object) override {
    auto reaction = entity_factory_->getReaction(json_object);  // entity_factory_->get<Reaction>(json_object);
    data_manager_->deleteReaction(reaction);
  }
};

#endif  // DELETEMESSAGEREACTIONHANDLER_H
