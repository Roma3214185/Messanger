#ifndef READMESSAGEHANDLER_H
#define READMESSAGEHANDLER_H

#include "Debug_profiling.h"
#include "interfaces/ISocketResponceHandler.h"
#include "usecases/messageusecase.h"

class ReadMessageHandler : public ISocketResponceHandler {
  DataManager *data_manager_;

 public:
  explicit ReadMessageHandler(DataManager *data_manager);
  void handle(const QJsonObject &json_object) override;
};

#endif  // READMESSAGEHANDLER_H
