#ifndef IMESSAGENETWORKMANAGER_H
#define IMESSAGENETWORKMANAGER_H

#include <nlohmann/json.hpp>

#include "Debug_profiling.h"
#include "interfaces/INetworkManagerBase.h"

class IMessageNetworkManager : public virtual INetworkManagerBase {
 public:
  virtual std::optional<long long> getChatIdOfMessage(long long message_id);
};

#endif  // IMESSAGENETWORKMANAGER_H
