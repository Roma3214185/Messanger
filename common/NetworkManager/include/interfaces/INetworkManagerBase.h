#ifndef INETWORKMANAGERBASE_H
#define INETWORKMANAGERBASE_H

#include <string>
#include <vector>

using UserId = int;
using Headers = std::vector<std::pair<std::string, std::string>>;

class INetworkManagerBase {
  public:
    virtual std::pair<int, std::string> forward(
        int                   port,
        const std::string&    body,
        const std::string&    path,
        const std::string&    method,
        const Headers&        extra_headers = {});
};

#endif // INETWORKMANAGERBASE_H
