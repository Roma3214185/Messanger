#ifndef BACKEND_NOTIFICATIONSERVICE_NOTIFICATIONMANAGER_NOTOFICATIONORCHESTRATOR_H_
#define BACKEND_NOTIFICATIONSERVICE_NOTIFICATIONMANAGER_NOTOFICATIONORCHESTRATOR_H_

#include "entities/Message.h"
#include "entities/MessageStatus.h"
#include "entities/Reaction.h"

class IPublisher;
class IUserSocketRepository;
class INetworkFacade;
class INotifier;

class NotificationOrchestrator {
  INetworkFacade *network_facade_;
  IPublisher* publisher_;
  INotifier* notifier_;
 public:

    NotificationOrchestrator(INetworkFacade *network_facade, IPublisher* publisher, INotifier* notifier);

    void onMessageStatusSaved(const std::string &payload);
    void onMessageSaved(const std::string &payload);
    void onMessageReactionDeleted(const std::string &payload);
    void onMessageReactionSaved(const std::string &payload);
    void onMessageDeleted(const std::string &payload);

 protected:
  std::vector<long long> fetchChatMembers(long long chat_id);
  std::optional<long long> getChatIdOfMessage(long long message_id);
};

#endif  // BACKEND_NOTIFICATIONSERVICE_NOTIFICATIONMANAGER_NOTOFICATIONORCHESTRATOR_H_
