#ifndef BACKEND_APIGATEWAY_SRC_PROXYCLIENT_PROXYCLIENT_H_
#define BACKEND_APIGATEWAY_SRC_PROXYCLIENT_PROXYCLIENT_H_

#include <crow.h>
#include <httplib.h>

#include <memory>
#include <nlohmann/json.hpp>

#include "ForwardRequestDTO.h"
#include "entities/RequestDTO.h"
#include "interfaces/IClient.h"

class ProxyClient {
  IClient *client_;

 public:
  explicit ProxyClient(IClient *client) : client_(client) {}
  NetworkResponse forward(const RequestDTO &, const int port);

 private:
  NetworkResponse makeRequest(const ForwardRequestDTO &, const std::string &method);
};

#endif  // BACKEND_APIGATEWAY_SRC_PROXYCLIENT_PROXYCLIENT_H_
