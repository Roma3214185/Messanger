#ifndef DELETEMESSAGEHANDLER_H
#define DELETEMESSAGEHANDLER_H

#include "interfaces/ISocketResponceHandler.h"

class IMessageDataManager;
class IMessageJsonService;

class DeleteMessageHandler : public ISocketResponceHandler {
 public:
  DeleteMessageHandler(IMessageJsonService *entity_factory,
                          IMessageDataManager *message_data_manager);
  void handle(const QJsonObject &json_object) override;

 private:
  IMessageJsonService *entity_factory_;
  IMessageDataManager *message_data_manager_;

};

#endif  // DELETEMESSAGEHANDLER_H
