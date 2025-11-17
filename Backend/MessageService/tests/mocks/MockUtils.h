#ifndef MOCKUTILS_H
#define MOCKUTILS_H

#include "Routes.h"

namespace MockUtils {

Routes getMockRoutes() {
  Routes mock_routes;
  mock_routes.messageSaved            = "test_message_saved";
  mock_routes.saveMessage             = "test_save_message";
  mock_routes.saveMessageStatus       = "test_save_message_status";
  mock_routes.messageSavedQueue       = "test_message_saved_queue";
  mock_routes.messageStatusSavedQueue = "test_message_status_saved_queue";
  mock_routes.saveMessageQueue        = "test_save_message_queue";
  mock_routes.saveMessageStatusQueue  = "test_save_message_status_queue";
  mock_routes.exchange                = "test_app.events";
  mock_routes.messageStatusSaved      = "test_message_status_saved";
  mock_routes.exchangeType            = "test_topic";
  return mock_routes;
}


}  // MockUtils

#endif // MOCKUTILS_H
