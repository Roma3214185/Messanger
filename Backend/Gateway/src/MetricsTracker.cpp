#include "MetricsTracker.h"

void MetricsTracker::startTimer(IMetrics* metrics) {
  metrics_ = metrics;
  start = std::chrono::steady_clock::now();
}

MetricsTracker::MetricsTracker(MetricsTracker&& other) noexcept : metrics_(other.metrics_), start(other.start) {
  other.metrics_ = nullptr;
}

MetricsTracker& MetricsTracker::operator=(MetricsTracker&& other) noexcept {
  if (this != &other) {
    if (metrics_) {
      auto end = std::chrono::steady_clock::now();
      double latency = std::chrono::duration<double>(end - start).count();
      metrics_->saveRequestLatency(latency);
    }

    metrics_ = other.metrics_;
    start = other.start;
    other.metrics_ = nullptr;
  }
  return *this;
}

MetricsTracker::~MetricsTracker() {
  if (!metrics_) return;

  auto end = std::chrono::steady_clock::now();
  double latency = std::chrono::duration<double>(end - start).count();
  metrics_->saveRequestLatency(latency);
}
