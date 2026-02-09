#ifndef PROXYCLIENT_H_
#define PROXYCLIENT_H_

#include <string>

class ForwardRequestDTO;
class RequestDTO;
class IClient;

using NetworkResponse = std::pair<int, std::string>;

class ProxyClient {
  IClient *client_;

 public:
  explicit ProxyClient(IClient *client) : client_(client) {}
  NetworkResponse forward(const RequestDTO &, const int port);

 private:
  NetworkResponse makeRequest(const ForwardRequestDTO &, const std::string &method);
};

#endif  // ROXYCLIENT_H_
