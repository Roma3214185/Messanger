#ifndef INETWORKMANAGER_H
#define INETWORKMANAGER_H

#include <string>
#include <vector>

using UserId = int;
using Headers = std::vector<std::pair<std::string, std::string>>;

class INetworkManager {
  public:
    std::pair<int, std::string> forward(
        int                   port,
        const std::string&    body,
        const std::string&    path,
        const std::string&    method,
        const Headers&        extra_headers = {});
};

#endif // INETWORKMANAGER_H
