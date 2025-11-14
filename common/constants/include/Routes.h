#ifndef ROUTES_H
#define ROUTES_H

#include <string>

struct Routes{
  std::string messageSaved       = "message_saved";
  std::string saveMessage        = "save_message";
  std::string saveMessageStatus  = "save_message_status";
  std::string notificationQueue  = "notification_service_queue";
  std::string exchange           = "app.events";
  std::string messageQueue       = "message_service_queue";
  std::string messageStatusSaved = "message_status_saved";
};

#endif // ROUTES_H
