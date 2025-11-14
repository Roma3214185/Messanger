#ifndef INETWORKMANAGERBASE_H
#define INETWORKMANAGERBASE_H

#include <string>
#include <vector>

#include "ProdConfigProvider.h"
#include "interfaces/IConfigProvider.h"

using UserId = int;
using Headers = std::vector<std::pair<std::string, std::string>>;

class INetworkManagerBase {
  public:
    explicit INetworkManagerBase(IConfigProvider* provider = &ProdConfigProvider::instance()) : provider_(provider) {}
    virtual std::pair<int, std::string> forward(
        int                   port,
        const std::string&    body,
        const std::string&    path,
        const std::string&    method,
        const Headers&        extra_headers = {});
  protected:
    IConfigProvider* provider_;
};

#endif // INETWORKMANAGERBASE_H
