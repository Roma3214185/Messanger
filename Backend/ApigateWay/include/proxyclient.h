#ifndef BACKEND_APIGATEWAY_SRC_PROXYCLIENT_PROXYCLIENT_H_
#define BACKEND_APIGATEWAY_SRC_PROXYCLIENT_PROXYCLIENT_H_

#include <crow.h>
#include <httplib.h>

#include <memory>
#include <nlohmann/json.hpp>

#include "interfaces/IClient.h"
#include "RequestDTO.h"
#include "ForwardRequestDTO.h"

class ProxyClient {
    IClient* client_;
  public:
    ProxyClient(IClient* client) : client_(client) { }
    NetworkResponse forward(const crow::request&, RequestDTO&, int port);

  private:
    NetworkResponse makeRequest(const ForwardRequestDTO&, const std::string& method);
};

#endif  // BACKEND_APIGATEWAY_SRC_PROXYCLIENT_PROXYCLIENT_H_
