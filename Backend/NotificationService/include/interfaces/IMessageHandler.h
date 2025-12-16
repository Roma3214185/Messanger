#ifndef IMESSAGEHANDLER_H
#define IMESSAGEHANDLER_H

#include <crow.h>

#include "notificationservice/managers/notificationmanager.h"

class IMessageHandler {
  public:
    virtual ~IMessageHandler() = default;

    virtual void handle(
        const crow::json::rvalue& message,
        std::shared_ptr<ISocket> socket,
        NotificationManager& manager
        ) = 0;
};

#endif // IMESSAGEHANDLER_H
