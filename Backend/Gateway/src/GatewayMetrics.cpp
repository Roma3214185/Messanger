#include "GatewayMetrics.h"

GatewayMetrics::GatewayMetrics(int port)
    : registry_(std::make_shared<prometheus::Registry>()),
      exposer_(std::make_unique<prometheus::Exposer>("127.0.0.1:" + std::to_string(port))),
      cache_hits_(
          prometheus::BuildCounter().Name("gateway_cache_hits_total").Help("Cache hit events").Register(*registry_)),
      cache_misses_(
          prometheus::BuildCounter().Name("gateway_cache_misses_total").Help("Cache miss events").Register(*registry_)),
      cache_store_(prometheus::BuildCounter()
                       .Name("gateway_cache_store_total")
                       .Help("Saving values to cache")
                       .Register(*registry_)),
      rl_hits_(prometheus::BuildCounter()
                   .Name("gateway_ratelimit_hits_total")
                   .Help("Rate limit triggered")
                   .Register(*registry_)),
      rl_allowed_(prometheus::BuildCounter()
                      .Name("gateway_ratelimit_allowed_total")
                      .Help("Rate limit passed")
                      .Register(*registry_)),
      backend_errors_(prometheus::BuildCounter()
                          .Name("gateway_backend_errors_total")
                          .Help("Backend request failed")
                          .Register(*registry_)),
      backend_timeout_(prometheus::BuildCounter()
                           .Name("gateway_backend_timeout_total")
                           .Help("Backend timeout")
                           .Register(*registry_)),
      backend_status_(prometheus::BuildCounter()
                          .Name("gateway_backend_status_total")
                          .Help("Backend status codes")
                          .Register(*registry_)),
      auth_ok_(prometheus::BuildCounter()
                   .Name("gateway_auth_ok_total")
                   .Help("Successful authentication")
                   .Register(*registry_)),
      auth_fail_(prometheus::BuildCounter()
                     .Name("gateway_auth_fail_total")
                     .Help("Failed authentication")
                     .Register(*registry_)),
      active_clients_family_(prometheus::BuildGauge()
                                 .Name("gateway_active_clients")
                                 .Help("Active connected clients")
                                 .Register(*registry_)),
      call_latency_family_(prometheus::BuildHistogram()
                               .Name("gateway_call_latency_seconds")
                               .Help("Time spent in backend calls")
                               .Register(*registry_)),
      request_counter_(prometheus::BuildCounter()
                           .Name("api_gateway_requests_total")
                           .Help("Total number of requests")
                           .Register(*registry_)),
      active_requests_family_(
          prometheus::BuildGauge().Name("gateway_active_requests").Help("Active requests").Register(*registry_)),
      request_size_hist_family_(prometheus::BuildHistogram()
                                    .Name("gateway_request_size_bytes")
                                    .Help("Size of incoming requests")
                                    .Register(*registry_)),
      response_size_hist_family_(prometheus::BuildHistogram()
                                     .Name("gateway_response_size_bytes")
                                     .Help("Size of outgoing responses")
                                     .Register(*registry_)),
      msg_size_histogram_family_(prometheus::BuildHistogram()
                                     .Name("gateway_message_size_bytes")
                                     .Help("Size of messages")
                                     .Register(*registry_)) {
  exposer_->RegisterCollectable(registry_);
  active_clients_ = &active_clients_family_.Add({});
  active_requests_ = &active_requests_family_.Add({});
  call_latency_ = &call_latency_family_.Add({}, prometheus::Histogram::BucketBoundaries(call_latency_buckets_));
  msg_size_histogram_ = &msg_size_histogram_family_.Add({{"direction", "incoming"}},
                                                        prometheus::Histogram::BucketBoundaries(msg_buckets_));
  response_size_hist_ = &response_size_hist_family_.Add({}, prometheus::Histogram::BucketBoundaries(response_buckets_));
  request_size_hist_ = &request_size_hist_family_.Add({}, prometheus::Histogram::BucketBoundaries(request_buckets_));
}

void GatewayMetrics::cacheHit(const std::string &path) { cache_hits_.Add({{"path", path}}).Increment(); }
void GatewayMetrics::cacheMiss(const std::string &path) { cache_misses_.Add({{"path", path}}).Increment(); }
void GatewayMetrics::cacheStats(bool wasHitted, const std::string &path) {
  wasHitted ? cacheHit(path) : cacheMiss(path);
}
void GatewayMetrics::cacheStore(const std::string &path) { cache_store_.Add({{"path", path}}).Increment(); }
void GatewayMetrics::newRequest(const std::string &path) {
  request_counter_.Add({{"path", path}}).Increment();
  active_requests_->Increment();
}

void GatewayMetrics::requestEnded(const std::string &path, int code, bool hitCache) {
  active_requests_->Decrement();
  cacheStats(hitCache, path);
  backendStatus(path, code);

  // check if code == rateLimitCode
  // check if code == InvalidAuthentification
}

void GatewayMetrics::rateLimitHit(const std::string &path, const std::string &key) {
  rl_hits_.Add({{"path", path}, {"key", key}}).Increment();
}

void GatewayMetrics::rateLimitAllowed(const std::string &path, const std::string &key) {
  rl_allowed_.Add({{"path", path}, {"key", key}}).Increment();
}

void GatewayMetrics::backendError(const std::string &path) { backend_errors_.Add({{"path", path}}).Increment(); }
void GatewayMetrics::backendTimeout(const std::string &path) { backend_timeout_.Add({{"path", path}}).Increment(); }
void GatewayMetrics::backendStatus(const std::string &path, int status) {
  backend_status_.Add({{"path", path}, {"status", std::to_string(status)}}).Increment();
}

void GatewayMetrics::authOk(const std::string &path) { auth_ok_.Add({{"path", path}}).Increment(); }
void GatewayMetrics::authFail(const std::string &path) { auth_fail_.Add({{"path", path}}).Increment(); }
void GatewayMetrics::userConnected() { active_clients_->Increment(); }
void GatewayMetrics::userDisconnected() { active_clients_->Decrement(); }
void GatewayMetrics::saveRequestLatency(const double latency) { call_latency_->Observe(latency); }

void GatewayMetrics::saveRequestSize(int size) { request_size_hist_->Observe(size); }
void GatewayMetrics::saveResponceSize(int size) { response_size_hist_->Observe(size); }

void GatewayMetrics::newMessage(const std::string &ip) {
  // TODO: count++;
  // TODO: save messages per user
  // TODO: save message size
}

void GatewayMetrics::saveMessageSize(int size) { msg_size_histogram_->Observe(size); }
