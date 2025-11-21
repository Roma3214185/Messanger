#ifndef BACKEND_APIGATEWAY_SRC_PROXYCLIENT_PROXYCLIENT_H_
#define BACKEND_APIGATEWAY_SRC_PROXYCLIENT_PROXYCLIENT_H_

#include <crow.h>
#include <httplib.h>

#include <memory>
#include <nlohmann/json.hpp>

#include "interfaces/IProxyClient.h"
#include "RequestDTO.h"

struct ProxyClient : public IProxyClient {
  NetworkResponse forward(const crow::request&, const RequestDTO&, int port) override;
};

#endif  // BACKEND_APIGATEWAY_SRC_PROXYCLIENT_PROXYCLIENT_H_
