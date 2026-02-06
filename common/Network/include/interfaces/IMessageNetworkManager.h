#ifndef IMESSAGENETWORKMANAGER_H
#define IMESSAGENETWORKMANAGER_H

#include <nlohmann/json.hpp>

#include "Debug_profiling.h"
#include "config/codes.h"
#include "config/ports.h"
#include "entities/ReactionInfo.h"

class IMessageNetworkManager {
 public:
  virtual ~IMessageNetworkManager() = default;
  virtual std::optional<long long> getChatIdOfMessage(long long message_id) = 0;
  virtual std::optional<ReactionInfo> getReaction(long long reaction_id) = 0;
  // todo: new service with reaction/gifs/images (?)
};

class ProxyClient;

class MessageNetworkManager : public IMessageNetworkManager {
public:
    MessageNetworkManager(ProxyClient* proxy);
    std::optional<long long> getChatIdOfMessage(long long message_id) override;
    std::optional<ReactionInfo> getReaction(long long reaction_id) override;
private:
    ProxyClient* proxy_;
};
#endif  // IMESSAGENETWORKMANAGER_H
