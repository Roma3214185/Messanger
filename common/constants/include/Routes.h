#ifndef ROUTES_H
#define ROUTES_H

#include <string>

struct Routes{
  std::string messageSaved            = "message_saved";
  std::string saveMessage             = "save_message";
  std::string saveMessageStatus       = "save_message_status";
  std::string saveMessageQueue        = "QueueSaveMessage";
  std::string saveMessageStatusQueue  = "QueueSaveMessageStatus";
  std::string messageSavedQueue       = "QueueMessageSaved";
  std::string messageStatusSavedQueue = "QueueMessageStatusSaved";
  std::string exchange                = "app.events";
  std::string messageStatusSaved      = "message_status_saved";
  std::string exchangeType            = "direct";
};

#endif // ROUTES_H
