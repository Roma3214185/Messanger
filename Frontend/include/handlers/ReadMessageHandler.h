#ifndef READMESSAGEHANDLER_H
#define READMESSAGEHANDLER_H

// auto json = QJsonObject{{"type", "read_message"},
//                         {"message_id", message.id},
//                         {"readed_by", current_user_id}};

#include "Debug_profiling.h"
#include "interfaces/ISocketResponceHandler.h"
#include "usecases/messageusecase.h"

class ReadMessageHandler : public ISocketResponceHandler {
  DataManager* data_manager_;

 public:
  ReadMessageHandler(DataManager* data_manager) : data_manager_(data_manager) {}

  void handle(const QJsonObject& json_object) override {
    if (!json_object.contains("message_id")) {
      LOG_ERROR("ReadMessageHandler doen't have field message_id");
      return;
    }

    if (!json_object.contains("receiver_id")) {
      LOG_ERROR("ReadMessageHandler doen't have field receiver_id");
      return;
    }

    long long message_id = json_object["message_id"].toInteger();
    long long readed_by  = json_object["receiver_id"].toInteger();
    data_manager_->readMessage(message_id, readed_by);
  }
};

#endif  // READMESSAGEHANDLER_H
