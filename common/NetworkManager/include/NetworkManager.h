#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include "interfaces/IUserNetworkManager.h"
#include "interfaces/IMessageNetworkManager.h"
#include "interfaces/IChatNetworkManager.h"

class NetworkManager : public IUserNetworkManager,
                      public IMessageNetworkManager,
                      public IChatNetworkManager {

};

#endif // NETWORKMANAGER_H
