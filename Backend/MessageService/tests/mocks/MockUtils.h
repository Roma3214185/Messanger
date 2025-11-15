#ifndef MOCKUTILS_H
#define MOCKUTILS_H

#include "Routes.h"

namespace MockUtils {

Routes getMockRoutes() {
  Routes mock_routes;
  mock_routes.messageSaved       = "test_message_saved";
  mock_routes.saveMessage        = "test_save_message";
  mock_routes.saveMessageStatus  = "test_save_message_status";
  mock_routes.notificationQueue  = "test_notification_service_queue";
  mock_routes.exchange           = "test_app.events";
  mock_routes.messageQueue       = "test_message_service_queue";
  mock_routes.messageStatusSaved = "test_message_status_saved";
  return mock_routes;
}


}  // MockUtils

#endif // MOCKUTILS_H
