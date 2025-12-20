#ifndef REALHTTPCLIENT_H
#define REALHTTPCLIENT_H

#include <httplib.h>

#include "interfaces/IClient.h"
#include "ForwardRequestDTO.h"
#include "RetryOptions.h"

constexpr int kBadGatewayCode = 502;
const std::string kBadGatewayMessage = "Bad Gateway: downstream no response";

class RealHttpClient : public IClient {
  public:
    NetworkResponse Get(const ForwardRequestDTO& request) override {
      auto client = setupClient(request.host_with_port);
      RetryOptions opts = getOptions(request);

      auto result = retryInvoke([&] {
        return client->Get(request.full_path, request.params, request.headers);
      }, opts);

      return getResponse(result);
    }

    NetworkResponse Delete(const ForwardRequestDTO& request) override {
      auto client = setupClient(request.host_with_port);
      RetryOptions opts = getOptions(request);

      auto result = retryInvoke([&] { // todo(roma): url_params??
        return client->Delete(request.full_path, request.headers);
      }, opts);

      return getResponse(result);
    }

    NetworkResponse Put(const ForwardRequestDTO& request) override {
      auto client = setupClient(request.host_with_port);
      RetryOptions opts = getOptions(request);

      auto result = retryInvoke([&] {  // todo(roma): url_params??
        return client->Put(request.full_path, request.headers, request.body, request.content_type);
      }, opts);

      return getResponse(result);
    }

    NetworkResponse Post(const ForwardRequestDTO& request) override {
      auto client = setupClient(request.host_with_port);
      RetryOptions opts = getOptions(request);

      auto result = retryInvoke([&] {  // todo(roma): url_params??
        return client->Post(request.full_path, request.headers, request.body, request.content_type);
      }, opts);

      return getResponse(result);
    }

  private:
    std::unique_ptr<httplib::Client> setupClient(const std::string& host_with_port) {
      auto client = std::make_unique<httplib::Client>(host_with_port);
      client->set_read_timeout(5, 0);
      client->set_connection_timeout(5, 0);
      return client;
    }

    RetryOptions getOptions(const ForwardRequestDTO& request) {
      RetryOptions opts;
      opts.max_attempts = request.times_retrying;
      opts.per_attempt_timeout = request.timeout;
      opts.retry_delay = std::chrono::milliseconds(100);
      opts.exponential_backoff = true;
      return opts;
    }

    NetworkResponse getResponse(std::optional<httplib::Result>& result) {
      if (!result || !result.value()) {
        return {kBadGatewayCode, kBadGatewayMessage};
      }

      auto& res = result.value().value();
      return { (int)res.status, res.body };
    }
};

#endif // REALHTTPCLIENT_H
