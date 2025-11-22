#ifndef REALHTTPCLIENT_H
#define REALHTTPCLIENT_H

#include <httplib.h>

#include "interfaces/IClient.h"
#include "ForwardRequestDTO.h"

constexpr int kBadGatewayCode = 502;
const std::string kBadGatewayMessage = "Bad Gateway: downstream no response";

class RealHttpClient : public IClient {
  public:
    NetworkResponse Get(const ForwardRequestDTO& request) override {
      auto client = std::make_unique<httplib::Client>(request.host_with_port.c_str());
      client->set_read_timeout(5, 0);
      client->set_connection_timeout(5, 0);
      httplib::Result res(std::unique_ptr<httplib::Response>(nullptr), httplib::Error::Unknown);
      res = client->Get(request.full_path, request.headers);
      if(!res) return {kBadGatewayCode, kBadGatewayMessage};
      return {static_cast<int>(res->status), res->body};
    }

    NetworkResponse Delete(const ForwardRequestDTO& request) override {
      auto client = std::make_unique<httplib::Client>(request.host_with_port.c_str());
      client->set_read_timeout(5, 0);
      client->set_connection_timeout(5, 0);
      httplib::Result res(std::unique_ptr<httplib::Response>(nullptr), httplib::Error::Unknown);
      res = client->Delete(request.full_path, request.headers);
      if(!res) return {kBadGatewayCode, kBadGatewayMessage};
      return {static_cast<int>(res->status), res->body};
    }

    NetworkResponse Put(const ForwardRequestDTO& request) override {
      auto client = std::make_unique<httplib::Client>(request.host_with_port.c_str());
      client->set_read_timeout(5, 0);
      client->set_connection_timeout(5, 0);
      httplib::Result res(std::unique_ptr<httplib::Response>(nullptr), httplib::Error::Unknown);
      res = client->Put(request.full_path, request.headers, request.body, request.content_type);
      if(!res) return {kBadGatewayCode, kBadGatewayMessage};
      return {static_cast<int>(res->status), res->body};
    }

    NetworkResponse Post(const ForwardRequestDTO& request) override {
      auto client = std::make_unique<httplib::Client>(request.host_with_port.c_str());
      client->set_read_timeout(5, 0);
      client->set_connection_timeout(5, 0);
      httplib::Result res(std::unique_ptr<httplib::Response>(nullptr), httplib::Error::Unknown);
      res = client->Post(request.full_path, request.headers, request.body, request.content_type);
      if(!res) return {kBadGatewayCode, kBadGatewayMessage};
      return {static_cast<int>(res->status), res->body};
    }
};

#endif // REALHTTPCLIENT_H
