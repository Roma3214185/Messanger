#ifndef IMESSAGENETWORKMANAGER_H
#define IMESSAGENETWORKMANAGER_H

#include <nlohmann/json.hpp>

#include "Debug_profiling.h"
#include "config/codes.h"
#include "config/ports.h"
#include "entities/ReactionInfo.h"
#include "interfaces/INetworkManagerBase.h"

class IMessageNetworkManager : public virtual INetworkManagerBase {
 public:
  virtual std::optional<long long> getChatIdOfMessage(long long message_id);
  virtual std::optional<ReactionInfo> getReaction(long long reaction_id); //todo: new service with reaction/gifs/images (?)
};

#endif  // IMESSAGENETWORKMANAGER_H
