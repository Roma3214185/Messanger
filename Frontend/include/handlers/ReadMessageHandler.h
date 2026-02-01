#ifndef READMESSAGEHANDLER_H
#define READMESSAGEHANDLER_H

#include "interfaces/ISocketResponceHandler.h"

class IMessageStatusJsonService;
class IMessageStatusDataManager;

class ReadMessageHandler : public ISocketResponceHandler {
  IMessageStatusJsonService *json_service_;
  IMessageStatusDataManager *data_manager_;

 public:
  explicit ReadMessageHandler(IMessageStatusJsonService *json_service, IMessageStatusDataManager *data_manager);
  void handle(const QJsonObject &json_object) override;
};

#endif  // READMESSAGEHANDLER_H
