#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include "interfaces/IChatNetworkManager.h"
#include "interfaces/IMessageNetworkManager.h"
#include "interfaces/IUserNetworkManager.h"

class NetworkManager : public IUserNetworkManager, public IMessageNetworkManager, public IChatNetworkManager {};

#endif  // NETWORKMANAGER_H
