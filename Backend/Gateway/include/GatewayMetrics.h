#ifndef GATEWAYMETRICS_H
#define GATEWAYMETRICS_H

#include <memory>

#include <prometheus/counter.h>
#include <prometheus/exposer.h>
#include <prometheus/histogram.h>
#include <prometheus/registry.h>
#include <prometheus/gauge.h>
#include <prometheus/registry.h>
#include "interfaces/IMetrics.h"
#include "MetricsTracker.h"

class GatewayMetrics : public IMetrics {
  public:
    explicit GatewayMetrics(int port)
        : registry_(std::make_shared<prometheus::Registry>())
        , exposer_(std::make_unique<prometheus::Exposer>("127.0.0.1:" + std::to_string(port)))
        , cache_hits_(prometheus::BuildCounter()
                        .Name("gateway_cache_hits_total")
                        .Help("Cache hit events")
                        .Register(*registry_))
        , cache_misses_(prometheus::BuildCounter()
                          .Name("gateway_cache_misses_total")
                          .Help("Cache miss events")
                          .Register(*registry_))
        , cache_store_(prometheus::BuildCounter()
                         .Name("gateway_cache_store_total")
                         .Help("Saving values to cache")
                         .Register(*registry_))
        ,rl_hits_(prometheus::BuildCounter()
                     .Name("gateway_ratelimit_hits_total")
                     .Help("Rate limit triggered")
                     .Register(*registry_))
        , rl_allowed_(prometheus::BuildCounter()
                        .Name("gateway_ratelimit_allowed_total")
                        .Help("Rate limit passed")
                        .Register(*registry_))
        , backend_errors_(prometheus::BuildCounter()
                            .Name("gateway_backend_errors_total")
                            .Help("Backend request failed")
                            .Register(*registry_))
        , backend_timeout_(prometheus::BuildCounter()
                             .Name("gateway_backend_timeout_total")
                             .Help("Backend timeout")
                             .Register(*registry_))
        , backend_status_(prometheus::BuildCounter()
                            .Name("gateway_backend_status_total")
                            .Help("Backend status codes")
                            .Register(*registry_))
        , auth_ok_(prometheus::BuildCounter()
                     .Name("gateway_auth_ok_total")
                     .Help("Successful authentication")
                     .Register(*registry_))
        , auth_fail_(prometheus::BuildCounter()
                       .Name("gateway_auth_fail_total")
                       .Help("Failed authentication")
                       .Register(*registry_))
        , active_clients_family_(prometheus::BuildGauge()
                                   .Name("gateway_active_clients")
                                   .Help("Active connected clients")
                                   .Register(*registry_))
        , call_latency_family_(prometheus::BuildHistogram()
                                 .Name("gateway_call_latency_seconds")
                                 .Help("Time spent in backend calls")
                                 .Register(*registry_))
        , request_counter_(prometheus::BuildCounter()
                                   .Name("api_gateway_requests_total")
                                   .Help("Total number of requests")
                                   .Register(*registry_))
        , active_requests_family_(prometheus::BuildGauge()
                                    .Name("gateway_active_requests")
                                    .Help("Active requests")
                                    .Register(*registry_))
        , request_size_hist_family_(prometheus::BuildHistogram()
                                .Name("gateway_request_size_bytes")
                                .Help("Size of incoming requests")
                                .Register(*registry_))
        , response_size_hist_family_(prometheus::BuildHistogram()
                                .Name("gateway_response_size_bytes")
                                .Help("Size of outgoing responses")
                                .Register(*registry_))
        , msg_size_histogram_family_(prometheus::BuildHistogram()
                                  .Name("gateway_message_size_bytes")
                                  .Help("Size of messages")
                                  .Register(*registry_))
    {
      exposer_->RegisterCollectable(registry_);
      active_clients_ = &active_clients_family_.Add({});
      active_requests_ = &active_requests_family_.Add({});
      call_latency_   = &call_latency_family_.Add({},prometheus::Histogram::BucketBoundaries(call_latency_buckets_));
      msg_size_histogram_ = &msg_size_histogram_family_.Add({{"direction","incoming"}}, prometheus::Histogram::BucketBoundaries(msg_buckets_));
      response_size_hist_ = &response_size_hist_family_.Add({}, prometheus::Histogram::BucketBoundaries(response_buckets_));
      request_size_hist_ = &request_size_hist_family_.Add({}, prometheus::Histogram::BucketBoundaries(request_buckets_));
    }

    void cacheHit(const std::string& path) { cache_hits_.Add({{"path", path}}).Increment(); }
    void cacheMiss(const std::string& path) { cache_misses_.Add({{"path", path}}).Increment(); }
    void cacheStats(bool wasHitted, const std::string& path) { wasHitted ? cacheHit(path) : cacheMiss(path); }
    void cacheStore(const std::string& path) { cache_store_.Add({{"path", path}}).Increment(); }
    void newRequest(const std::string& path) {
      request_counter_.Add({{"path", path}}).Increment();
      active_requests_->Increment();
    }

    void requestEnded(const std::string& path, int code, bool hitCache) override {
      active_requests_->Decrement();
      cacheStats(hitCache, path);
      backendStatus(path, code);

      //check if code == rateLimitCode
      //check if code == InvalidAuthentification
    }

    void rateLimitHit(const std::string& path, const std::string& key) {
      rl_hits_.Add({{"path", path}, {"key", key}}).Increment();
    }

    void rateLimitAllowed(const std::string& path, const std::string& key) {
      rl_allowed_.Add({{"path", path}, {"key", key}}).Increment();
    }

    void backendError(const std::string& path) { backend_errors_.Add({{"path", path}}).Increment(); }
    void backendTimeout(const std::string& path) { backend_timeout_.Add({{"path", path}}).Increment(); }
    void backendStatus(const std::string& path, int status) {
      backend_status_.Add({{"path", path}, {"status", std::to_string(status)}}).Increment();
    }

    void authOk(const std::string& path) { auth_ok_.Add({{"path", path}}).Increment(); }
    void authFail(const std::string& path) { auth_fail_.Add({{"path", path}}).Increment(); }
    void userConnected() { active_clients_->Increment(); }
    void userDisconnected() { active_clients_->Decrement(); }
    void saveRequestLatency(const double latency) { call_latency_->Observe(latency); }

    void saveRequestSize(int size) {
      request_size_hist_->Observe(size);
    }
    void saveResponceSize(int size) {
      response_size_hist_->Observe(size);
    }

    void newMessage(const std::string& ip) override {
      //TODO: count++;
      //TODO: save messages per user
      //TODO: save message size
    }

    void saveMessageSize(int size) {
      msg_size_histogram_->Observe(size);
    }

  private:
    std::shared_ptr<prometheus::Registry> registry_;
    std::unique_ptr<prometheus::Exposer> exposer_;
    prometheus::Family<prometheus::Counter>& cache_hits_;
    prometheus::Family<prometheus::Counter>& cache_misses_;
    prometheus::Family<prometheus::Counter>& cache_store_;
    prometheus::Family<prometheus::Counter>& rl_hits_;
    prometheus::Family<prometheus::Counter>& rl_allowed_;
    prometheus::Family<prometheus::Counter>& backend_errors_;
    prometheus::Family<prometheus::Counter>& backend_timeout_;
    prometheus::Family<prometheus::Counter>& backend_status_;
    prometheus::Family<prometheus::Counter>& auth_ok_;
    prometheus::Family<prometheus::Counter>& auth_fail_;
    prometheus::Family<prometheus::Counter>&  request_counter_;
    prometheus::Family<prometheus::Gauge>& active_clients_family_;
    prometheus::Family<prometheus::Gauge>& active_requests_family_;
    prometheus::Family<prometheus::Histogram>& call_latency_family_;
    prometheus::Family<prometheus::Histogram>& request_size_hist_family_;
    prometheus::Family<prometheus::Histogram>& response_size_hist_family_;
    prometheus::Family<prometheus::Histogram>& msg_size_histogram_family_;
    prometheus::Gauge* active_clients_;
    prometheus::Gauge* active_requests_;
    prometheus::Histogram* call_latency_;
    prometheus::Histogram* request_size_hist_;
    prometheus::Histogram* response_size_hist_;
    prometheus::Histogram* msg_size_histogram_;

    const std::vector<double> request_buckets_ = {100, 500, 1024, 2048, 4096, 10'000, 100'000, 400'000, 1'000'000, 5'000'000};
    const std::vector<double> response_buckets_ = {100, 500, 1024, 2048, 4096, 10'000, 100'000, 400'000, 1'000'000, 5'000'000};
    const std::vector<double> msg_buckets_ = {100, 500, 1000, 5000, 10000, 50000, 100000, 500000, 1000000};
    const std::vector<double> call_latency_buckets_ = {0.001, 0.005, 0.01, 0.05, 0.1, 0.5, 1, 2, 5};
};

#endif // GATEWAYMETRICS_H
