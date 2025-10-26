#ifndef MESSAGEWORKER_H
#define MESSAGEWORKER_H

#include "../../RabbitMQClient/rabbitmqclient.h"
#include "../../GenericRepository/GenericReposiroty.h"
#include "../Headers/Message.h"
#include "../MessageManager/MessageManager.h"
#include "../NotificationManager/notificationmanager.h"

// class MessageWorker
// {
// public:
//     MessageWorker(RabbitMQClient& mq, MessageManager& manager, NotificationManager& notifService);

//     void handleEvent(const std::string& msg);

// private:
//     RabbitMQClient& mq_;
//     MessageManager& msgManager;
//     NotificationManager& notifService;
// };

#endif // MESSAGEWORKER_H
