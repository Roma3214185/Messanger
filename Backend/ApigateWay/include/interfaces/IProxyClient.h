#ifndef IPROXYCLIENT_H
#define IPROXYCLIENT_H

#include <string>
#include <crow.h>

#include "RequestDTO.h"

class IProxyClient {
  public:
    using ResponseCode = int;
    using ResponseBody = std::string;
    using NetworkResponse = std::pair<ResponseCode, ResponseBody>;

    virtual ~IProxyClient() = default;
    virtual NetworkResponse forward(const crow::request&, const RequestDTO&, int port) = 0;
};

#endif // IPROXYCLIENT_H
