//#include "messageworker.h"

// MessageWorker::MessageWorker(RabbitMQClient& mq, MessageManager& manager, NotificationManager& notifService)
//     : mq_(mq)
//     , msgManager(manager)
//     , notifService(notifService)
// {
//     mq_.subscribe("message_events", [this](const std::string& msg){
//         handleEvent(msg);
//     });
// }

// void MessageWorker::handleEvent(const std::string& msg) {
//     try {
//         auto json_event = nlohmann::json::parse(msg);
//         if(json_event["type"] == "message_read") {
//             MessageStatus status{
//                 .id = json_event["message_id"],
//                 .receiver_id = json_event["reader_id"],
//                 .is_read = true,
//                 .read_at = QDateTime::currentDateTime().toSecsSinceEpoch()
//             };
//             msgManager.saveMessageStatus(status);
//             auto chatId = msgManager.getChatId(status.id);
//             if(!chatId) {
//                 LOG_ERROR("Chat id not found for message id '{}'", status.id);
//                 return;
//             }
//             notifService.notifyMessageRead(*chatId, status);

//         }
//     } catch (const std::exception& e) {
//         LOG_ERROR("Failed to handle RabbitMQ message: {}", e.what());
//     }
// }
