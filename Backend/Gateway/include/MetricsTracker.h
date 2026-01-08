#ifndef METRICSTRACKER_H
#define METRICSTRACKER_H

#include <chrono>

#include "interfaces/IMetrics.h"

struct MetricsTracker {
  IMetrics* metrics_ = nullptr;
  std::chrono::steady_clock::time_point start;
  MetricsTracker() = default;

  void startTimer(IMetrics* metrics) {
    metrics_ = metrics;
    start = std::chrono::steady_clock::now();
  }

  MetricsTracker(const MetricsTracker&) = delete;
  MetricsTracker& operator=(const MetricsTracker&) = delete;

  MetricsTracker(MetricsTracker&& other) noexcept : metrics_(other.metrics_), start(other.start) {
    other.metrics_ = nullptr;
  }

  MetricsTracker& operator=(MetricsTracker&& other) noexcept {
    if (this != &other) {
      if (metrics_) {
        using namespace std::chrono;
        auto end = std::chrono::steady_clock::now();
        auto latency = duration<double>(end - start).count();
        metrics_->saveRequestLatency(latency);
      }

      metrics_ = other.metrics_;
      start = other.start;
      other.metrics_ = nullptr;
    }
    return *this;
  }

  ~MetricsTracker() {
    if (!metrics_) return;
    using namespace std::chrono;

    auto end = steady_clock::now();
    auto latency = duration<double>(end - start).count();
    metrics_->saveRequestLatency(latency);
  }
};

#endif  // METRICSTRACKER_H
