#ifndef GATEWAYMETRICS_H
#define GATEWAYMETRICS_H

#include <prometheus/counter.h>
#include <prometheus/exposer.h>
#include <prometheus/gauge.h>
#include <prometheus/histogram.h>
#include <prometheus/registry.h>

#include <memory>

#include "MetricsTracker.h"
#include "interfaces/IMetrics.h"

class GatewayMetrics : public IMetrics {
 public:
  explicit GatewayMetrics(int port);

  void cacheHit(const std::string &path);
  void cacheMiss(const std::string &path);
  void cacheStats(bool wasHitted, const std::string &path);
  void cacheStore(const std::string &path);

  void newRequest(const std::string &path) override;
  void requestEnded(const std::string &path, int code, bool hitCache) override;

  void rateLimitHit(const std::string &path, const std::string &key);
  void rateLimitAllowed(const std::string &path, const std::string &key);

  void backendError(const std::string &path);
  void backendTimeout(const std::string &path);
  void backendStatus(const std::string &path, int status);

  void authOk(const std::string &path);
  void authFail(const std::string &path); //todo: group related funcitons in 1
  void userConnected() override;
  void userDisconnected() override;
  void saveRequestLatency(const double latency) override;

  void saveRequestSize(int size);
  void saveResponceSize(int size);

  void newMessage(const std::string &ip) override;
  void saveMessageSize(int size);

 private:
  std::shared_ptr<prometheus::Registry> registry_;
  std::unique_ptr<prometheus::Exposer> exposer_;
  prometheus::Family<prometheus::Counter> &cache_hits_;
  prometheus::Family<prometheus::Counter> &cache_misses_;
  prometheus::Family<prometheus::Counter> &cache_store_;
  prometheus::Family<prometheus::Counter> &rl_hits_;
  prometheus::Family<prometheus::Counter> &rl_allowed_;
  prometheus::Family<prometheus::Counter> &backend_errors_;
  prometheus::Family<prometheus::Counter> &backend_timeout_;
  prometheus::Family<prometheus::Counter> &backend_status_;
  prometheus::Family<prometheus::Counter> &auth_ok_;
  prometheus::Family<prometheus::Counter> &auth_fail_;
  prometheus::Family<prometheus::Counter> &request_counter_;
  prometheus::Family<prometheus::Gauge> &active_clients_family_;
  prometheus::Family<prometheus::Gauge> &active_requests_family_;
  prometheus::Family<prometheus::Histogram> &call_latency_family_;
  prometheus::Family<prometheus::Histogram> &request_size_hist_family_;
  prometheus::Family<prometheus::Histogram> &response_size_hist_family_;
  prometheus::Family<prometheus::Histogram> &msg_size_histogram_family_;
  prometheus::Gauge *active_clients_;
  prometheus::Gauge *active_requests_;
  prometheus::Histogram *call_latency_;
  prometheus::Histogram *request_size_hist_;
  prometheus::Histogram *response_size_hist_;
  prometheus::Histogram *msg_size_histogram_;

  const std::vector<double> request_buckets_ = {100,    500,     1024,    2048,      4096,
                                                10'000, 100'000, 400'000, 1'000'000, 5'000'000};
  const std::vector<double> response_buckets_ = {100,    500,     1024,    2048,      4096,
                                                 10'000, 100'000, 400'000, 1'000'000, 5'000'000};
  const std::vector<double> msg_buckets_ = {100, 500, 1000, 5000, 10000, 50000, 100000, 500000, 1000000};
  const std::vector<double> call_latency_buckets_ = {0.001, 0.005, 0.01, 0.05, 0.1, 0.5, 1, 2, 5};
};

#endif  // GATEWAYMETRICS_H
