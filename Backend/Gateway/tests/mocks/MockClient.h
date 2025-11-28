#ifndef MOCKPROXY_H
#define MOCKPROXY_H

#include "interfaces/IClient.h"

struct MockClient : public IClient {
    ForwardRequestDTO last_request;
    int call_get = 0;
    int call_delete = 0;
    int call_put = 0;
    int call_post = 0;
    NetworkResponse mock_response;

    NetworkResponse Get(const ForwardRequestDTO& request) override {
      last_request = request;
      ++call_get;
      return mock_response;
    }

    NetworkResponse Delete(const ForwardRequestDTO& request) override {
      last_request = request;
      ++call_delete;
       return mock_response;
    }

    NetworkResponse Put(const ForwardRequestDTO& request) override {
      last_request = request;
      ++call_put;
       return mock_response;
    }

    NetworkResponse Post(const ForwardRequestDTO& request) override {
      last_request = request;
      ++call_post;
      return mock_response;
    }
};

#endif // MOCKPROXY_H
