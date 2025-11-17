#ifndef MOCKUTILS_H
#define MOCKUTILS_H

#include "Routes.h"
#include "codes.h"

namespace MockUtils {

inline Routes getMockRoutes() {
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

inline StatusCodes getMockCodes() {
  StatusCodes status_codes;
  status_codes.success = 1200;
  status_codes.serverError = 2500;
  status_codes.userError = 1400;

  status_codes.invalidToken = "test_Invalid or expired token";

  return status_codes;
}


}  // MockUtils

#endif // MOCKUTILS_H
