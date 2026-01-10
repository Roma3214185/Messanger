#ifndef ROUTES_H
#define ROUTES_H

#include <string>

namespace Config::Routes {
static constexpr const char *messageSaved = "message_saved";
static constexpr const char *messageDeleted = "message_deleted";
static constexpr const char *saveMessage = "save_message";
static constexpr const char *saveMessageStatus = "save_message_status";
static constexpr const char *saveMessageQueue = "QueueSaveMessage";
static constexpr const char *saveMessageStatusQueue = "QueueSaveMessageStatus";
static constexpr const char *messageSavedQueue = "QueueMessageSaved";
static constexpr const char *messageStatusSavedQueue = "QueueMessageStatusSaved";
static constexpr const char *exchange = "app.events";
static constexpr const char *messageStatusSaved = "message_status_saved";
static constexpr const char *sendRequest = "send_request";
static constexpr const char *exchangeType = "direct";
}  // namespace Config::Routes

#endif  // ROUTES_H
