#ifndef METRICSTRACKER_H
#define METRICSTRACKER_H

#include <chrono>

#include "interfaces/IMetrics.h"

struct MetricsTracker {
  IMetrics *metrics_ = nullptr;
  std::chrono::steady_clock::time_point start;

  void startTimer(IMetrics *metrics) {
    metrics_ = metrics;
    start = std::chrono::steady_clock::now();
  }

  ~MetricsTracker() {
    if (!metrics_)
      return;
    using namespace std::chrono;

    auto end = steady_clock::now();
    auto latency = duration<double>(end - start).count();
    metrics_->saveRequestLatency(latency);
  }
};

#endif // METRICSTRACKER_H
