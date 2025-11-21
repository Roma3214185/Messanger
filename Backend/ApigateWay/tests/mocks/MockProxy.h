#ifndef MOCKPROXY_H
#define MOCKPROXY_H

#include "interfaces/IProxyClient.h"

struct MockProxy : public IProxyClient {
    int last_port;
    RequestDTO last_request_info;
    crow::request last_request;
    NetworkResponse mock_response;

    int call_forward = 0;

    NetworkResponse forward(const crow::request& req, const RequestDTO& info, int port) override {
      ++call_forward;
      last_port = port;
      last_request_info = info;
      last_request = req;
      return mock_response;
    }
};

#endif // MOCKPROXY_H
