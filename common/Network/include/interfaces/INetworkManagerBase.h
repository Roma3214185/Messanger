#ifndef INETWORKMANAGERBASE_H
#define INETWORKMANAGERBASE_H

#include <string>
#include <vector>

using UserId = long long;
using Headers = std::vector<std::pair<std::string, std::string>>;

class INetworkManagerBase {
 public:
  virtual std::pair<int, std::string> forward(  // todo(roma): alias
      int port, const std::string &body, const std::string &path, const std::string &method,
      const Headers &extra_headers = {}); //todo: reqDto
  virtual ~INetworkManagerBase() = default;
};

#endif  // INETWORKMANAGERBASE_H
